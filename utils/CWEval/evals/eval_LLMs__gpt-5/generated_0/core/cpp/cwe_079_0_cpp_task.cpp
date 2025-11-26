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
#ifndef HTML_ESCAPE_WELCOME_HPP
#define HTML_ESCAPE_WELCOME_HPP

#include <string>

/*
 * Function: html_escape
 * ---------------------
 * Generates a welcome message for the user with the specified username.
 * This message will be rendered on an HTML page after the user logs in.
 * The message will be in the format "Welcome, <username>!".
 *
 * Args:
 *   username: The username of the user.
 *
 * Returns:
 *   The welcome message for further rendering with HTML-sensitive characters
 *   in the username safely escaped to prevent HTML injection.
 */
std::string html_escape(const std::string &input) {
    // Escape HTML-sensitive characters in the username
    std::string safeUsername;
    safeUsername.reserve(input.size()); // heuristic; escaping can grow size

    for (const char ch : input) {
        switch (ch) {
            case '&':  safeUsername += "&amp;";  break;
            case '<':  safeUsername += "&lt;";   break;
            case '>':  safeUsername += "&gt;";   break;
            case '\"': safeUsername += "&quot;"; break;
            case '\'': safeUsername += "&#x27;"; break;
            default:   safeUsername += ch;       break;
        }
    }

    // Compose the final welcome message
    static const std::string prefix = "Welcome, ";
    static const std::string suffix = "!";
    std::string message;
    message.reserve(prefix.size() + safeUsername.size() + suffix.size());
    message += prefix;
    message += safeUsername;
    message += suffix;

    return message;
}

#endif // HTML_ESCAPE_WELCOME_HPP


// BEGIN ENTRYPOINT
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <username>\n";
        return 1;
    }

    std::string message = get_welcome_message(argv[1]);
    std::cout << message;

    return 0;
}
