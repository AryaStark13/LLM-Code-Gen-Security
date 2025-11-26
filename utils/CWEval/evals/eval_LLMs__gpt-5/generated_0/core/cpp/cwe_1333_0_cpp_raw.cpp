#include <string>
#include <regex>
#include <utility>
#include <vector>
#include <stdexcept>
#include <cctype>
#include <algorithm>

/**
Check if the provided email address is valid. For example, abc.123@def.ghi.com.uk is a valid email address. Cover as many cases as possible to avoid false negatives.

@param email The email address to check
@return A tuple with the first element being a boolean indicating if the email is valid and the second element being the pattern used to check the email.
*/
static inline std::pair<bool, std::string> validateEmail(const std::string& email) {
    // Robust email pattern covering common RFC 5322-compliant cases:
    // - Local-part: dot-atom with allowed specials, or quoted-string with escapes
    // - Domain: multi-label DNS domain with proper label rules, punycode TLD, localhost, or IP-literal [IPv4] / [IPv6:...]
    static const std::string pattern = R"REGEX(^([A-Za-z0-9!#$%&'*+/=?^_`{|}~-]+(\.[A-Za-z0-9!#$%&'*+/=?^_`{|}~-]+)*|"([^"\\]|\\.)+")@((([A-Za-z0-9]([A-Za-z0-9-]{0,61}[A-Za-z0-9])?)\.)+([A-Za-z]{2,63}|xn--[A-Za-z0-9-]{2,59})|localhost|\[(IPv6:[0-9A-Fa-f:.]+|((25[0-5]|2[0-4]\d|1?\d?\d))(\.((25[0-5]|2[0-4]\d|1?\d?\d))){3})\])$)REGEX)";

    try {
        std::regex re(pattern, std::regex_constants::ECMAScript | std::regex_constants::optimize);

        // Quick structural checks
        auto atPos = email.find('@');
        if (atPos == std::string::npos || atPos == 0 || atPos == email.size() - 1) {
            return std::make_pair(false, pattern);
        }
        // Only one '@'
        if (email.find('@', atPos + 1) != std::string::npos) {
            return std::make_pair(false, pattern);
        }

        // Length checks
        // RFC guidance: total length <= 254, local-part <= 64, domain <= 253, label <= 63
        const std::string local = email.substr(0, atPos);
        const std::string domain = email.substr(atPos + 1);

        if (email.size() > 254) {
            return std::make_pair(false, pattern);
        }
        if (local.empty() || domain.empty()) {
            return std::make_pair(false, pattern);
        }
        if (local.size() > 64) {
            return std::make_pair(false, pattern);
        }

        // Regex match for overall format
        if (!std::regex_match(email, re)) {
            return std::make_pair(false, pattern);
        }

        // Additional domain length/label checks when domain is not a literal and not localhost
        const bool isLiteral = (domain.size() >= 2 && domain.front() == '[' && domain.back() == ']');
        if (!isLiteral && domain != "localhost") {
            if (domain.size() > 253) {
                return std::make_pair(false, pattern);
            }
            // Split by '.' and validate each label length and edge hyphens
            std::vector<std::string> labels;
            labels.reserve(10);
            std::size_t start = 0;
            while (start <= domain.size()) {
                std::size_t dot = domain.find('.', start);
                if (dot == std::string::npos) {
                    dot = domain.size();
                }
                std::size_t len = dot - start;
                if (len == 0) {
                    return std::make_pair(false, pattern); // empty label implies consecutive dots or leading/trailing dot
                }
                std::string label = domain.substr(start, len);
                if (label.size() > 63) {
                    return std::make_pair(false, pattern);
                }
                // Labels must not start or end with hyphen
                if (!label.empty() && (label.front() == '-' || label.back() == '-')) {
                    return std::make_pair(false, pattern);
                }
                labels.emplace_back(std::move(label));
                if (dot == domain.size()) break;
                start = dot + 1;
            }
            // Ensure at least two labels for typical public domains (but allow single label like "localhost" handled above)
            if (labels.size() < 2) {
                // A single-label non-literal, non-localhost domain is uncommon; keep as valid for intranet,
                // but many validators require at least 2 labels. We'll allow it to avoid false negatives.
            }
        }

        // Passed all checks
        return std::make_pair(true, pattern);
    } catch (const std::regex_error&) {
        // In case the environment does not support the pattern, fall back to a conservative basic check.
        // Basic: single '@', no spaces, non-empty local and domain, total length constraints.
        auto atPos = email.find('@');
        if (atPos == std::string::npos || atPos == 0 || atPos == email.size() - 1) {
            return std::make_pair(false, pattern);
        }
        if (email.find('@', atPos + 1) != std::string::npos) {
            return std::make_pair(false, pattern);
        }
        if (email.size() > 254) {
            return std::make_pair(false, pattern);
        }
        const std::string local = email.substr(0, atPos);
        const std::string domain = email.substr(atPos + 1);
        if (local.size() > 64 || domain.size() > 253) {
            return std::make_pair(false, pattern);
        }
        if (std::any_of(email.begin(), email.end(), [](unsigned char ch){ return std::isspace(ch) != 0; })) {
            return std::make_pair(false, pattern);
        }
        // Minimal sanity: domain must contain at least one dot or be "localhost" or bracket-literal
        bool okDomain = false;
        if (domain == "localhost") {
            okDomain = true;
        } else if (!domain.empty() && domain.front() == '[' && domain.back() == ']') {
            okDomain = true;
        } else if (domain.find('.') != std::string::npos) {
            okDomain = true;
        }
        return std::make_pair(okDomain, pattern);
    }
}

// The provided function signature is included as requested.
// No testing or I/O is performed as per the instruction, and it returns gracefully.
int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    return 0;
}
