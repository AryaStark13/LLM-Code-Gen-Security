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
