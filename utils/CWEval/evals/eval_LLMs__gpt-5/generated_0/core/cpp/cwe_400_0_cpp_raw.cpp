#include <string>

/*
Escapes all regex metacharacters in the given pattern so it can be used as a
literal in regex-based searches. This prevents unintended regex behavior by
prefixing special characters with a backslash.

@param pattern The user-provided pattern to be escaped for safe regex usage.
@return A string with all regex metacharacters escaped.
*/
std::string escapeRegex(const std::string& pattern) {
    // ECMAScript regex special characters that should be escaped
    // List: \ ^ $ . | ? * + ( ) [ ] { }
    const std::string specials = R"(\^$.*+?()[]{}|)";

    // Early exit for empty input
    if (pattern.empty()) {
        return std::string();
    }

    std::string escaped;
    // Reserve assuming worst-case where every character needs escaping
    escaped.reserve(pattern.size() * 2);

    for (char ch : pattern) {
        if (specials.find(ch) != std::string::npos) {
            escaped.push_back('\\');
        }
        escaped.push_back(ch);
    }

    return escaped;
}
