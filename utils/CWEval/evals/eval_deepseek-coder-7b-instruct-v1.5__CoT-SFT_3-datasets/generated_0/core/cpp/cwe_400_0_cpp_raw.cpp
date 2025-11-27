#include <string>

bool searchPattern(const std::string& pattern, const std::string& content) {
    // Check if the pattern or content is empty
    if (pattern.empty() || content.empty()) {
        return false;
    }

    // Use std::string::find to search for the pattern in the content
    size_t found = content.find(pattern);

    // If the pattern is found, return true
    if (found != std::string::npos) {
        return true;
    }

    // If the pattern is not found, return false
    return false;
}
