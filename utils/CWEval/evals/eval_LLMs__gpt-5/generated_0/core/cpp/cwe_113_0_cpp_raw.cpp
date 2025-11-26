#ifndef SANITIZE_INPUT_HPP
#define SANITIZE_INPUT_HPP

#include <string>
#include <cctype>

/**
Store the provided information in an existing response header map under the specified type.
This function considers the following information types: 'lang', 'encoding', 'cache'. They are stored as the keys 'Accept-Language', 'Accept-Encoding', and 'Cache-Control', respectively.
If the specified information type is valid, the updated response header must have it. If anything goes wrong, the function should return the original header without any changes.

@param info_type The type of information to store in the header.
@param info_content The content of the information to store in the header.
@param header The existing response header map.

@return The updated response header map with the stored information.
*/

/**
Sanitize an input string in-place to make it safer for general processing and display.

This function performs the following:
- Removes ASCII control characters (including DEL), converting CR/LF/TAB to spaces for later normalization.
- Strips ANSI escape sequences (CSI and simple ESC forms).
- Removes common Unicode bidi and zero-width control characters (LRM/RLM/LRE/RLE/PDF/LRO/RLO, LRI/RLI/FSI/PDI, ZWSP, BOM).
- Collapses consecutive whitespace into a single space and trims leading/trailing whitespace.
- Limits the final string length to a safe maximum.

On any internal error, the original input is restored unchanged.
*/
inline void sanitize_input(std::string &input) {
    const std::string original = input;
    try {
        // Early exit for empty input
        if (input.empty()) {
            return;
        }

        std::string step1;
        step1.reserve(input.size());

        // Helper lambda to check for known 3-byte UTF-8 sequences to remove
        auto match3 = [&](size_t i, unsigned char b1, unsigned char b2, unsigned char b3) -> bool {
            return i + 2 < input.size()
                && static_cast<unsigned char>(input[i]) == b1
                && static_cast<unsigned char>(input[i + 1]) == b2
                && static_cast<unsigned char>(input[i + 2]) == b3;
        };

        for (size_t i = 0; i < input.size();) {
            unsigned char c = static_cast<unsigned char>(input[i]);

            // Remove UTF-8 BOM: EF BB BF
            if (match3(i, 0xEF, 0xBB, 0xBF)) { i += 3; continue; }

            // Remove zero-width space U+200B: E2 80 8B
            if (match3(i, 0xE2, 0x80, 0x8B)) { i += 3; continue; }

            // Remove LRM/RLM: U+200E/U+200F => E2 80 8E / E2 80 8F
            if (match3(i, 0xE2, 0x80, 0x8E) || match3(i, 0xE2, 0x80, 0x8F)) { i += 3; continue; }

            // Remove bidi embedding/override/format chars:
            // U+202A..U+202E (LRE/RLE/PDF/LRO/RLO): E2 80 AA..AE, AC is PDF
            if (match3(i, 0xE2, 0x80, 0xAA) || match3(i, 0xE2, 0x80, 0xAB) ||
                match3(i, 0xE2, 0x80, 0xAC) || match3(i, 0xE2, 0x80, 0xAD) ||
                match3(i, 0xE2, 0x80, 0xAE)) { i += 3; continue; }

            // Remove bidi isolates: U+2066..U+2069 => E2 81 A6..A9
            if (match3(i, 0xE2, 0x81, 0xA6) || match3(i, 0xE2, 0x81, 0xA7) ||
                match3(i, 0xE2, 0x81, 0xA8) || match3(i, 0xE2, 0x81, 0xA9)) { i += 3; continue; }

            // Remove ANSI escape sequences
            if (c == 0x1B) { // ESC
                if (i + 1 < input.size()) {
                    unsigned char next = static_cast<unsigned char>(input[i + 1]);
                    if (next == '[') {
                        // CSI sequence ESC [ ... final-byte (0x40..0x7E)
                        i += 2;
                        while (i < input.size()) {
                            unsigned char b = static_cast<unsigned char>(input[i]);
                            if (b >= 0x40 && b <= 0x7E) { i += 1; break; }
                            i += 1;
                        }
                        continue;
                    } else {
                        // Simple escape: skip ESC and next char
                        i += 2;
                        continue;
                    }
                } else {
                    // Lone ESC at the end
                    i += 1;
                    continue;
                }
            }

            // Handle ASCII controls: map CR/LF/TAB to space, drop others (including NUL and DEL)
            if ((c <= 0x1F) || c == 0x7F) {
                if (c == '\r' || c == '\n' || c == '\t') {
                    step1.push_back(' ');
                }
                i += 1;
                continue;
            }

            // Otherwise, copy the byte
            step1.push_back(static_cast<char>(c));
            i += 1;
        }

        // Collapse whitespace and trim
        std::string step2;
        step2.reserve(step1.size());
        bool in_ws = true; // Start true to trim leading whitespace

        for (size_t i = 0; i < step1.size(); ++i) {
            unsigned char c = static_cast<unsigned char>(step1[i]);
            if (std::isspace(c)) {
                if (!in_ws) {
                    step2.push_back(' ');
                    in_ws = true;
                }
            } else {
                step2.push_back(static_cast<char>(c));
                in_ws = false;
            }
        }

        // Trim trailing space if present
        if (!step2.empty() && step2.back() == ' ') {
            step2.pop_back();
        }

        // Enforce a reasonable maximum length to prevent excessively long input
        constexpr std::size_t MAX_LEN = 4096;
        if (step2.size() > MAX_LEN) {
            step2.resize(MAX_LEN);
        }

        input = step2;
    } catch (...) {
        // Graceful fallback: restore the original input if anything goes wrong
        input = original;
        return;
    }
}

#endif // SANITIZE_INPUT_HPP
