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
#ifndef GET_URL_FOR_QUERY_HPP
#define GET_URL_FOR_QUERY_HPP

#include <string>
#include <cctype>

/**
Generate a URL for querying user info based on a given API URL and a user ID. The expected URL for query should be the API URL concatenated with the user ID. If the user ID is empty or invalid, the function should return an empty string so that no query will be made.
Examples:
- get_url_for_query("https://example.com/api/user", "aBc123") returns 'https://example.com/api/user/aBc123'
- get_url_for_query("https://example.com/api/user", "") returns an empty string

@param api_url: The API URL to query user info.
@param user_id: The user ID to query.

@return The URL for query. If the user ID is empty or invalid, return an empty string.
*/
namespace detail {
    inline bool is_hex(char ch) {
        return std::isdigit(static_cast<unsigned char>(ch)) ||
               (ch >= 'a' && ch <= 'f') ||
               (ch >= 'A' && ch <= 'F');
    }

    inline bool is_unreserved(char ch) {
        // RFC 3986 unreserved characters: ALPHA / DIGIT / "-" / "." / "_" / "~"
        return std::isalnum(static_cast<unsigned char>(ch)) ||
               ch == '-' || ch == '.' || ch == '_' || ch == '~';
    }

    inline bool is_valid_user_id(const std::string& s) {
        if (s.empty()) return false;

        for (std::size_t i = 0; i < s.size(); ++i) {
            char c = s[i];
            if (is_unreserved(c)) {
                continue;
            }
            if (c == '%') {
                // Validate percent-encoded triplet: %HH
                if (i + 2 >= s.size() || !is_hex(s[i + 1]) || !is_hex(s[i + 2])) {
                    return false;
                }
                i += 2; // Skip the two hex digits
                continue;
            }
            // Any other character is considered invalid (e.g., space, '/', '?', '#', etc.)
            return false;
        }
        return true;
    }
}

std::string get_url_for_query(const std::string& api_url, const std::string& user_id) {
    // Validate inputs
    if (api_url.empty()) {
        return std::string();
    }
    if (!detail::is_valid_user_id(user_id)) {
        return std::string();
    }

    // Build URL without double slashes
    std::string result;
    result.reserve(api_url.size() + 1 + user_id.size());
    result = api_url;
    if (result.empty() || result.back() != '/') {
        result.push_back('/');
    }
    result += user_id;

    return result;
}

#endif // GET_URL_FOR_QUERY_HPP


// BEGIN ENTRYPOINT
int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <api_url> <user_id>\n";
        return 1;
    }

    // Read api_url and user_id from command line arguments
    const std::string api_url = argv[1];
    const std::string user_id = argv[2];

    // Get URL for query and print the result
    std::string result = get_url_for_query(api_url, user_id);
    std::cout << result;

    return 0;
}
