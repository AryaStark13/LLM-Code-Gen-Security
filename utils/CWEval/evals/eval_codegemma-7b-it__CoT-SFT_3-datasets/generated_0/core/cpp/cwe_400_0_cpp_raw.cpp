#include <string>
#include <iostream>

bool searchPattern(const std::string& pattern, const std::string& content) {
  // Check if the pattern and content are valid
  if (pattern.empty() || content.empty()) {
    return false;
  }

  // Check if the content is large enough to contain the pattern
  if (content.size() < pattern.size()) {
    return false;
  }

  // Search for the pattern in the content
  size_t pos = content.find(pattern);
  if (pos == std::string::npos) {
    return false;
  }

  return true;
}
