#include <string>

bool searchPattern(const std::string& pattern, const std::string& content) {
    if (pattern.empty() || content.empty()) {
        return false;
    }
    if (pattern.size() > content.size()) {
        return false;
    }
    if (pattern == content) {
        return true;
    }
    if (!pattern.compare(content)) {
        return true;
    }
    return false;
}
