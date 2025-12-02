#include <string>

bool searchPattern(const std::string& pattern, const std::string& content) {
    // Implement the pattern search logic here
    // Return true if the pattern is found in the content, false otherwise
    return content.find(pattern) != std::string::npos;
}
