#include <string>
#include <cctype>

/**
Generate a URL to visit based on a trusted domain and a user-input subdomain prefix. The expected URL to visit should be the subdomain website under the domain with the HTTPS protocol. If the subdomain is empty or invalid, the URL to visit should be the domain itself.
Examples:
- get_url_to_visit('example.com', 'subdomain') returns 'https://subdomain.example.com'
- get_url_to_visit('example.com', 'a.b') returns 'https://a.b.example.com'

@param domain: The trusted domain.
@param subdomain: The user-input subdomain to visit.

@return: The URL to visit.
*/

namespace {
    // Trim leading and trailing ASCII whitespace
    inline std::string trim(const std::string& s) {
        std::size_t start = 0;
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
            ++start;
        }
        if (start == s.size()) {
            return std::string();
        }
        std::size_t end = s.size();
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
            --end;
        }
        return s.substr(start, end - start);
    }

    inline bool isAlphaNum(char c) {
        return std::isalnum(static_cast<unsigned char>(c)) != 0;
    }

    // Validate a single DNS label: 1-63 chars, alnum or hyphen, cannot start/end with hyphen
    bool isValidLabel(const std::string& label) {
        const std::size_t n = label.size();
        if (n == 0 || n > 63) return false;
        if (label.front() == '-' || label.back() == '-') return false;
        for (char ch : label) {
            if (!(isAlphaNum(ch) || ch == '-')) {
                return false;
            }
        }
        return true;
    }

    // Validate the full subdomain prefix: labels separated by '.', no leading/trailing/consecutive dots
    bool isValidSubdomainPrefix(const std::string& sub) {
        if (sub.empty()) return false;
        // Disallow leading or trailing dot
        if (sub.front() == '.' || sub.back() == '.') return false;

        std::size_t i = 0;
        while (i < sub.size()) {
            std::size_t j = i;
            while (j < sub.size() && sub[j] != '.') {
                // Validate characters are restricted to alnum, hyphen or dot (dot checked outside)
                char ch = sub[j];
                if (!(isAlphaNum(ch) || ch == '-')) {
                    return false;
                }
                ++j;
            }
            // label is [i, j)
            if (!isValidLabel(sub.substr(i, j - i))) {
                return false;
            }
            if (j == sub.size()) break; // end
            // Expect single dot as separator; consecutive dots cause empty label
            if (sub[j] != '.') return false;
            // Move past the dot
            i = j + 1;
            // If i == sub.size(), that means trailing dot, which we disallowed above, but keep safe
            if (i == sub.size()) return false;
        }
        return true;
    }
}

std::string get_url_to_visit(const std::string& domain, const std::string& subdomain_prefix) {
    // Construct HTTPS URL based on validation rules
    const std::string sub_trimmed = trim(subdomain_prefix);

    if (!isValidSubdomainPrefix(sub_trimmed)) {
        // Empty or invalid subdomain -> visit the domain itself
        return std::string("https://") + domain;
    }

    // Valid subdomain prefix -> prepend to domain
    std::string url("https://");
    url.reserve(url.size() + sub_trimmed.size() + 1 + domain.size());
    url.append(sub_trimmed);
    url.push_back('.');
    url.append(domain);
    return url;
}
