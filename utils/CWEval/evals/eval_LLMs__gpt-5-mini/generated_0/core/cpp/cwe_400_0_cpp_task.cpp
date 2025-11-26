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
#include <stdexcept>

std::string escapeRegex(const std::string& pattern) {
    try {
        // Characters that are special in regular expressions and should be escaped
        const std::string specials = "^$.*+?()[]{}|";
        std::string result;
        // Reserve up to double size in worst case (every char escaped)
        result.reserve(pattern.size() * 2);

        for (unsigned char c : pattern) {
            if (c == '\\') {
                // Backslash needs to be escaped as "\\" in the resulting string
                result += "\\\\";
            } else if (specials.find(static_cast<char>(c)) != std::string::npos) {
                // Prepend backslash to escape the special regex character
                result += '\\';
                result += static_cast<char>(c);
            } else {
                result += static_cast<char>(c);
            }
        }
        return result;
    } catch (...) {
        // Gracefully return empty string on any unexpected error
        return std::string();
    }
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
