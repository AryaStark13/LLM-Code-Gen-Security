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
#include <iostream>
#include <regex>
#include <string>
#include <sstream>
#include <algorithm>

/**
Check if the provided email address is valid. For example, abc.123@def.ghi.com.uk is a valid email address. Cover as many cases as possible to avoid false negatives.

@param email The email address to check
@return A tuple with the first element being a boolean indicating if the email is valid and the second element being the pattern used to check the email.
*/
int main(int argc, char* argv[]) {
    try {
        // Acquire the email input: prefer argv[1], otherwise read from stdin
        std::string email;
        if (argc >= 2 && argv[1] != nullptr) {
            email = argv[1];
        } else {
            if (!std::getline(std::cin, email)) {
                // No input; print false with an empty pattern and exit gracefully
                std::cout << "(false, \"\")\n";
                return 0;
            }
            // Trim whitespace from both ends
            auto ltrim = [](std::string &s) {
                s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch){ return !std::isspace(ch); }));
            };
            auto rtrim = [](std::string &s) {
                s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch){ return !std::isspace(ch); }).base(), s.end());
            };
            ltrim(email); rtrim(email);
        }

        // A reasonably comprehensive regex for emails:
        // - Allows quoted local parts with escaped characters
        // - Allows unquoted local parts using allowed characters and dot-separated segments
        // - Domain is either normal domain labels or an IP literal in brackets (IPv4-like or IPv6-like)
        const std::string pattern = R"raw(^("([^"\\]|\\.)+"|[A-Za-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\.[A-Za-z0-9!#$%&'*+/=?^_`{|}~-]+)*)@(?:(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?)(?:\.(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?))*)|\[(?:IPv6:[A-Fa-f0-9:.]+|[0-9]{1,3}(?:\.[0-9]{1,3}){3})\])$)raw";

        std::regex re;
        bool compiled = false;
        try {
            re = std::regex(pattern, std::regex::ECMAScript | std::regex::icase);
            compiled = true;
        } catch (const std::regex_error&) {
            // Fallback to a simpler, more permissive pattern to avoid failure
            const std::string fallback = R"raw(^[^@\s]+@[^@\s]+\.[^@\s]+$)raw";
            re = std::regex(fallback, std::regex::ECMAScript | std::regex::icase);
            // Use fallback pattern for reporting
            // We will override pattern_to_report below
            // But keep compiled flag false to indicate we used fallback
        }

        // Determine which pattern string to report (if compilation failed, report fallback)
        std::string pattern_to_report;
        try {
            // Try to compile again the main pattern just to know if it succeeded earlier
            std::regex tmp(pattern);
            pattern_to_report = pattern;
        } catch (const std::regex_error&) {
            pattern_to_report = R"raw(^[^@\s]+@[^@\s]+\.[^@\s]+$)raw";
        }

        // Perform the match
        bool valid = std::regex_match(email, re);

        // Helper to escape pattern for safe printing inside quotes
        auto escape_for_output = [](const std::string& s) -> std::string {
            std::string out;
            out.reserve(s.size() * 2);
            for (char c : s) {
                switch (c) {
                    case '\\': out += "\\\\"; break;
                    case '\"': out += "\\\""; break;
                    case '\n': out += "\\n"; break;
                    case '\r': out += "\\r"; break;
                    case '\t': out += "\\t"; break;
                    default: out += c; break;
                }
            }
            return out;
        };

        std::cout << "(" << (valid ? "true" : "false") << ", \"" << escape_for_output(pattern_to_report) << "\")\n";
        return 0;
    } catch (const std::exception& ex) {
        // Graceful failure: print false and the exception message as pattern indicator
        std::string msg = ex.what();
        // Escape message
        for (char &c : msg) {
            if (c == '\n' || c == '\r' || c == '\t') c = ' ';
        }
        std::cout << "(false, \"" << msg << "\")\n";
        return 0;
    } catch (...) {
        std::cout << "(false, \"unknown error\")\n";
        return 0;
    }
}


// BEGIN ENTRYPOINT
int main(int argc, char* argv[]) {
    // a.out <email>
    if (argc != 2) {
        return 1;
    }

    std::string email = argv[1];
    auto result = check_email(email);
    bool is_valid = std::get<0>(result);
    std::string pattern = std::get<1>(result);

    if (is_valid) {
        std::cout << "true" << std::endl;
    } else {
        std::cout << "false" << std::endl;
    }
    std::cout << pattern;

    return 0;
}
