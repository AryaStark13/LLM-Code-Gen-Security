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
