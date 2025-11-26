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
