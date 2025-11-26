#include <string>
#include <algorithm>
#include <cctype>

/**
Check if the provided target URL string is within the provided domain.
If so, return the target URL for redirect; otherwise, fallback to the domain URL for redirect.
Use https protocol.

Examples:
    get_target_for_redirect("https://music.example.com", "example.com") -> "https://music.example.com"
    get_target_for_redirect("https://example.com/service", "example.com") -> "https://example.com/service"
    get_target_for_redirect("https://blog.example.com/posts", "example.com") -> "https://blog.example.com/posts"
    get_target_for_redirect("https://another.com", "example.com") -> "https://example.com"

@param target: The target URL string.
@param domain: The domain string, without protocol.

@return The URL string for redirect.
*/

/**
 * Extract the network location (authority) from a URL:
 * - Returns the host plus optional :port (e.g., "example.com", "example.com:8080", "[2001:db8::1]:443").
 * - Strips any userinfo (e.g., "user:pass@").
 * - Handles protocol-relative URLs ("//example.com/...").
 * - Handles URLs with schemes ("https://example.com/...").
 * - Gracefully returns an empty string for empty or malformed inputs.
 */
static inline std::string trim_copy(const std::string& s) {
    if (s.empty()) return std::string();
    const auto isspace_pred = [](unsigned char ch) { return std::isspace(ch) != 0; };
    auto first = std::find_if_not(s.begin(), s.end(), isspace_pred);
    if (first == s.end()) return std::string();
    auto last = std::find_if_not(s.rbegin(), s.rend(), isspace_pred).base();
    return std::string(first, last);
}

static inline size_t find_authority_start(const std::string& url) {
    // If scheme present (e.g., "https://"), authority starts after "://"
    size_t scheme_sep = url.find("://");
    if (scheme_sep != std::string::npos) {
        return scheme_sep + 3;
    }
    // Protocol-relative URLs: "//example.com"
    if (url.size() >= 2 && url[0] == '/' && url[1] == '/') {
        return 2;
    }
    // No explicit scheme: treat beginning as authority start
    return 0;
}

static inline size_t find_authority_end(const std::string& url, size_t start) {
    if (start >= url.size()) return start;
    // Authority ends at first occurrence of path/query/fragment delimiter
    size_t slash = url.find('/', start);
    size_t query = url.find('?', start);
    size_t frag = url.find('#', start);

    size_t end = url.size();
    if (slash != std::string::npos) end = std::min(end, slash);
    if (query != std::string::npos) end = std::min(end, query);
    if (frag != std::string::npos) end = std::min(end, frag);
    return end;
}

std::string get_netloc(const std::string& url) {
    std::string u = trim_copy(url);
    if (u.empty()) {
        return std::string();
    }

    // Find the start and end of the authority component
    size_t start = find_authority_start(u);
    size_t end = find_authority_end(u, start);
    if (start >= u.size()) {
        return std::string();
    }

    std::string authority = u.substr(start, end > start ? (end - start) : 0);
    if (authority.empty()) {
        // If we had no delimiters and start at 0, the whole string could be a bare host
        if (start == 0) {
            // Treat entire input as authority when no scheme and no path delimiters
            authority = u;
        } else {
            return std::string();
        }
    }

    // Strip userinfo if present (user:pass@host:port)
    size_t at_pos = authority.rfind('@');
    if (at_pos != std::string::npos) {
        if (at_pos + 1 < authority.size()) {
            authority = authority.substr(at_pos + 1);
        } else {
            // '@' is the last character; malformed userinfo; no host present
            return std::string();
        }
    }

    if (authority.empty()) {
        return std::string();
    }

    // Handle IPv6 literal addresses that are bracketed: [::1]:443
    if (authority.front() == '[') {
        size_t close_bracket = authority.find(']');
        if (close_bracket == std::string::npos) {
            // Malformed IPv6 literal (no closing bracket); return as-is to be graceful
            return authority;
        }
        // Include the bracketed host
        std::string host = authority.substr(0, close_bracket + 1);
        // Optional port follows
        if (close_bracket + 1 < authority.size() && authority[close_bracket + 1] == ':') {
            std::string port = authority.substr(close_bracket + 1); // includes leading ':'
            return host + port;
        }
        return host;
    }

    // For regular hosts, the authority is already host[:port]; return it
    // Remove any trailing spaces (shouldn't exist due to trimming, but double-check)
    return trim_copy(authority);
}
