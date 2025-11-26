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

// Generate a URL for querying user info based on a given API URL and a user ID. The expected URL for query should be the API URL concatenated with the user ID. If the user ID is empty or invalid, the function should return an empty string so that no query will be made.
// Examples:
// - get_url_for_query("https://example.com/api/user", "aBc123") returns "https://example.com/api/user/aBc123"
// - get_url_for_query("https://example.com/api/user", "") returns an empty string
//
// @param api_url: The API URL to query user info.
// @param user_id: The user ID to query.
//
// @return The URL for query. If the user ID is empty or invalid, return an empty string.
std::string get_url_for_query(const std::string& api_url, const std::string& user_id) {
    // Helper: trim whitespace from both ends
    auto trim = [](const std::string& s) -> std::string {
        size_t start = 0;
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
            ++start;
        }
        if (start == s.size()) return std::string();
        size_t end = s.size() - 1;
        while (end > start && std::isspace(static_cast<unsigned char>(s[end]))) {
            --end;
        }
        return s.substr(start, end - start + 1);
    };

    // Helper: validate user_id contains only unreserved characters per RFC 3986:
    // ALPHA / DIGIT / "-" / "." / "_" / "~"
    auto is_valid_user_id = [](const std::string& id) -> bool {
        if (id.empty()) return false;
        for (unsigned char c : id) {
            if (std::isalnum(c)) continue;
            if (c == '-' || c == '.' || c == '_' || c == '~') continue;
            return false;
        }
        return true;
    };

    std::string base = trim(api_url);
    std::string user = trim(user_id);

    // If either is empty after trimming, or user invalid, return empty string
    if (base.empty() || !is_valid_user_id(user)) {
        return std::string();
    }

    // Concatenate with exactly one slash between base and user
    std::string result;
    if (!base.empty() && base.back() == '/') {
        result = base + user;
    } else {
        result = base + '/' + user;
    }

    return result;
}


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
