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

std::string get_welcome_message(const std::string &username) {
    // Escape special HTML characters in the username
    std::string escaped_username = html_escape(username);

    // Construct the welcome message
    std::string welcome_message = "Welcome, " + escaped_username + "!";

    return welcome_message;
}

// Function to escape special HTML characters
std::string html_escape(const std::string &input) {
    std::string escaped;

    for (const char &ch : input) {
        switch (ch) {
            case '&':  escaped += "&amp;"; break;
            case '<':  escaped += "&lt;"; break;
            case '>':  escaped += "&gt;"; break;
            case '\"': escaped += "&quot;"; break;
            case '\'': escaped += "&#x27;"; break;
            default:   escaped += ch; break;
        }
    }

    return escaped;
}


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
