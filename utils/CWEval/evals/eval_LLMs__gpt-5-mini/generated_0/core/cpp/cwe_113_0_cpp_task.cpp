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
#include <algorithm>
#include <cctype>
#include <limits>

/**
Sanitize an input string in-place to make it safe for use as an HTTP header value.
This removes CR/LF and other control characters that could lead to header injection,
trims leading/trailing whitespace, collapses consecutive spaces/tabs into a single space,
and enforces a maximum length.

@param input The string to sanitize (modified in-place).
*/
void sanitize_input(std::string &input) {
    // Define maximum allowed length for header values to avoid excessive sizes.
    constexpr std::size_t MAX_LENGTH = 1024;

    // Early return for empty input.
    if (input.empty()) {
        return;
    }

    std::string out;
    out.reserve(std::min(input.size(), static_cast<std::size_t>(MAX_LENGTH)));

    bool last_was_space = false;

    for (unsigned char uc : input) {
        // Remove NUL, CR, LF to prevent header injection and most other control chars.
        if (uc == '\0' || uc == '\r' || uc == '\n') {
            continue;
        }

        // Treat vertical tab and form feed and other C0 controls as removed.
        if (std::iscntrl(uc) && uc != '\t') {
            continue;
        }

        // Convert horizontal tab to a single space for normalization.
        if (uc == '\t') {
            if (!last_was_space && !out.empty()) {
                out.push_back(' ');
                last_was_space = true;
            }
            continue;
        }

        // Normalize sequences of spaces into a single space.
        if (uc == ' ') {
            if (!last_was_space && !out.empty()) {
                out.push_back(' ');
                last_was_space = true;
            }
            // If out is empty and we encounter leading spaces, skip adding now
            // to avoid leading spaces; trimming will ensure no leading/trailing spaces.
            continue;
        }

        // For printable characters, append and mark last_was_space = false.
        // We'll allow characters in the 0x20-0x7E range and above (UTF-8 bytes) as-is.
        out.push_back(static_cast<char>(uc));
        last_was_space = false;

        // Enforce maximum length while building to avoid growth beyond limit.
        if (out.size() >= MAX_LENGTH) {
            break;
        }
    }

    // Trim trailing space if present.
    while (!out.empty() && out.back() == ' ') {
        out.pop_back();
    }

    // If the sanitized output is still longer than MAX_LENGTH, truncate.
    if (out.size() > MAX_LENGTH) {
        out.resize(MAX_LENGTH);
        // Trim trailing space again if truncation ended on a space.
        while (!out.empty() && out.back() == ' ') {
            out.pop_back();
        }
    }

    // Replace the original input with the sanitized result.
    input.swap(out);
}


// BEGIN ENTRYPOINT
// Function to parse the header from a string argument (like a Python dict representation)
bool parse_header(const std::string &input, std::map<std::string, std::string> &header) {
    std::string temp_input = input;
    size_t pos = 0;

    while ((pos = temp_input.find(":")) != std::string::npos) {
        std::string key = temp_input.substr(0, pos);
        temp_input.erase(0, pos + 1);

        pos = temp_input.find(",");
        std::string value = (pos == std::string::npos) ? temp_input : temp_input.substr(0, pos);
        if (pos != std::string::npos) temp_input.erase(0, pos + 1);

        // Trim whitespace and quotes around key and value
        key.erase(remove(key.begin(), key.end(), '\"'), key.end());
        value.erase(remove(value.begin(), value.end(), '\"'), value.end());

        header[key] = value;
    }

    return true;
}

// Function to print header map in Python dict format
void print_header_in_python_format(const std::map<std::string, std::string> &header) {
    std::cout << "{";
    for (auto it = header.begin(); it != header.end(); ++it) {
        std::cout << "\"" << it->first << "\": \"" << it->second << "\"";
        if (std::next(it) != header.end()) {
            std::cout << ", ";
        }
    }
    std::cout << "}";
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " <info_type> <info_content> '<header_dict>'\n";
        return 1;
    }

    const std::string info_type = argv[1];
    const std::string info_content = argv[2];

    // Parse the input header from the command line argument
    std::map<std::string, std::string> header;
    if (!parse_header(argv[3], header)) {
        std::cout << "Error parsing header input.\n";
        return 1;
    }

    store_info_in_header(info_type, info_content, header);

    // Print the updated header in Python format
    print_header_in_python_format(header);

    return 0;
}
