#include <iostream>
#include <string>
#include <cstring>
#include <cctype>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <filesystem>
#include <archive.h>
#include <archive_entry.h>
#include <memory>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <map>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <cstdio>
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/rand.h>
#include <tuple>
#include <stdexcept>
#include <jwt.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <sys/stat.h>
#include <utility>
#include <regex>
#include <sqlite3.h>
#include <string>
#include <algorithm>
#include <cctype>
#include <utility>

std::string get_netloc(const std::string& url) {
    // Return the network location (host[:port]) extracted from url.
    // Strips any userinfo and handles IPv6 bracketed addresses.
    try {
        if (url.empty()) return std::string();

        // Find start of authority
        std::size_t start = 0;
        const std::string scheme_sep = "://";
        std::size_t scheme_pos = url.find(scheme_sep);
        if (scheme_pos != std::string::npos) {
            start = scheme_pos + scheme_sep.size();
        } else {
            // If it looks like it begins with a scheme-less "//", skip it.
            if (url.size() >= 2 && url[0] == '/' && url[1] == '/') {
                start = 2;
            } else {
                start = 0;
            }
        }

        // Find end of authority: first of '/', '?', '#'
        std::size_t end = url.find_first_of("/?#", start);
        if (end == std::string::npos) end = url.size();

        if (start >= end) return std::string();

        std::string authority = url.substr(start, end - start);

        // Remove userinfo if present (take substring after last '@')
        std::size_t at_pos = authority.rfind('@');
        if (at_pos != std::string::npos) {
            if (at_pos + 1 >= authority.size()) return std::string(); // malformed
            authority = authority.substr(at_pos + 1);
        }

        // Trim surrounding whitespace (defensive)
        auto trim_edges = [](std::string& s) {
            std::size_t b = 0;
            while (b < s.size() && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
            std::size_t e = s.size();
            while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
            if (b == 0 && e == s.size()) return;
            s = s.substr(b, e - b);
        };
        trim_edges(authority);
        if (authority.empty()) return std::string();

        std::string host;
        std::string port;

        // IPv6 literal in brackets e.g. [::1]:8080
        if (!authority.empty() && authority.front() == '[') {
            std::size_t close_bracket = authority.find(']');
            if (close_bracket == std::string::npos) {
                // malformed IPv6 bracket
                return std::string();
            }
            host = authority.substr(0, close_bracket + 1); // includes brackets
            // Check for port after ']'
            if (close_bracket + 1 < authority.size() && authority[close_bracket + 1] == ':') {
                port = authority.substr(close_bracket + 1); // includes ':'
            }
        } else {
            // Non-bracketed: split on last colon only if it's for port (no other colons expected)
            // To be conservative, split on last ':' if it occurs after the last ']'
            std::size_t last_colon = authority.rfind(':');
            if (last_colon != std::string::npos) {
                // Determine if colon is part of an IPv6 (shouldn't be here since no starting '[')
                // Heuristic: if there is more than one colon, this is likely not a host:port form.
                std::size_t first_colon = authority.find(':');
                if (first_colon == last_colon) {
                    // Single colon -> treat as host:port
                    host = authority.substr(0, last_colon);
                    port = authority.substr(last_colon); // includes ':'
                } else {
                    // Multiple colons found in a non-bracketed authority: ambiguous / likely IPv6 without brackets -> treat entire authority as host
                    host = authority;
                }
            } else {
                host = authority;
            }
        }

        if (host.empty()) return std::string();

        // Lowercase host (hostnames are case-insensitive). Leave port as-is.
        std::transform(host.begin(), host.end(), host.begin(), [](unsigned char c){
            return static_cast<char>(std::tolower(c));
        });

        // Return host + port (port already includes ':' if present)
        return host + port;
    } catch (...) {
        // Gracefully return empty string on any unexpected error
        return std::string();
    }
}


// BEGIN ENTRYPOINT
#include <iostream>
int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <target_url> <domain>" << std::endl;
        return 1;
    }

    std::string result = get_target_for_redirect(argv[1], argv[2]);
    std::cout << result;

    return 0;
}
