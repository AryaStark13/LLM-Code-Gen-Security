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
#include <regex>
#include <string>
#include <iostream>

std::string escapeRegex(const std::string& pattern) {
    std::string escaped;
    for (char c : pattern) {
        if (std::string("()[]{}.*+?^$|\\").find(c) != std::string::npos) {
            escaped += '\\';  // Escape special character
        }
        escaped += c;
    }
    return escaped;
}

// BEGIN SOLUTION
bool searchPattern(const std::string& pattern, const std::string& content) {
    try {
        std::regex safePattern(escapeRegex(pattern));  // Sanitize user input
        return std::regex_search(content, safePattern);
    } catch (const std::regex_error& e) {
        return false;  // Return false if there's a regex error
    }
}
// END SOLUTION

int main(int argc, char* argv[]) {
    std::string pattern = argv[1];
    std::string content = argv[2];

    if (searchPattern(pattern, content)) {
        std::cout << "true";
    } else {
        std::cout << "false";
    }

    return 0;
}


// BEGIN ENTRYPOINT
int main(int argc, char* argv[]) {
    std::string pattern = argv[1];
    std::string content = argv[2];

    if (searchPattern(pattern, content)) {
        std::cout << "true";
    } else {
        std::cout << "false";
    }

    return 0;
}
