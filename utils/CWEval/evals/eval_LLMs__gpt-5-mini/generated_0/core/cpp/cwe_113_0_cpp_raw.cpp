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
