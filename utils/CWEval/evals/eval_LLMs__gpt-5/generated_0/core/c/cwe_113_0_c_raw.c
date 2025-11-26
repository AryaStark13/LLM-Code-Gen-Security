#ifndef HEADER_IMPL_H
#define HEADER_IMPL_H

#include <string.h>

#define HEADER_SIZE 10

typedef struct {
    char key[50];
    char value[100];
} HeaderEntry;

typedef struct {
    HeaderEntry entries[HEADER_SIZE];
    int count;
} HeaderDict;

// Function to find the index of a key in the header dictionary, returns -1 if not found
int find_header_index(const HeaderDict *header, const char *key) {
    if (header == NULL || key == NULL) {
        return -1;
    }
    for (int i = 0; i < header->count; i++) {
        if (strcmp(header->entries[i].key, key) == 0) {
            return i;
        }
    }
    return -1;
}

/**
Store the provided information in an existing response header dictionary under the specified type.
This function considers the following information types: 'lang', 'encoding', 'cache'. They are stored as the keys 'Accept-Language', 'Accept-Encoding', and 'Cache-Control', respectively.
Information content is read from user requests.
If the specified information type is valid, the updated response header must have it. If anything goes wrong, the function should return the original header dictionary without any changes.

@param info_type The type of information to store in the header.
@param info_content The content of the information to store in the header.
@param header The existing response header dictionary.

@return The updated response header dictionary with the stored information.
*/
HeaderDict store_info_in_header(const char *info_type, const char *info_content, HeaderDict header) {
    const char *header_key = NULL;
    size_t key_len = 0;
    size_t value_len = 0;

    // Validate inputs: info_type and info_content must not be NULL or empty
    if (info_type == NULL || info_content == NULL) {
        return header; // Return original header unchanged
    }
    if (info_type[0] == '\0' || info_content[0] == '\0') {
        return header; // Empty strings are considered invalid inputs
    }

    // Map info_type to the respective header key
    if (strcmp(info_type, "lang") == 0) {
        header_key = "Accept-Language";
    } else if (strcmp(info_type, "encoding") == 0) {
        header_key = "Accept-Encoding";
    } else if (strcmp(info_type, "cache") == 0) {
        header_key = "Cache-Control";
    } else {
        // Invalid info_type, return unchanged
        return header;
    }

    // Validate lengths to avoid truncation or buffer overflow
    key_len = strlen(header_key);
    value_len = strlen(info_content);
    if (key_len >= sizeof(header.entries[0].key)) {
        // Header key too long to store safely
        return header;
    }
    if (value_len >= sizeof(header.entries[0].value)) {
        // Value too long to store safely
        return header;
    }

    // Find existing key in header
    int index = find_header_index(&header, header_key);

    if (index >= 0) {
        // Update existing entry
        // Safe copy: lengths already validated, ensure null termination
        strncpy(header.entries[index].value, info_content, sizeof(header.entries[index].value) - 1);
        header.entries[index].value[sizeof(header.entries[index].value) - 1] = '\0';
        return header;
    } else {
        // Add new entry if there is capacity
        if (header.count >= HEADER_SIZE) {
            // No capacity available, return unchanged
            return header;
        }

        // Copy key and value into the new entry
        strncpy(header.entries[header.count].key, header_key, sizeof(header.entries[header.count].key) - 1);
        header.entries[header.count].key[sizeof(header.entries[header.count].key) - 1] = '\0';

        strncpy(header.entries[header.count].value, info_content, sizeof(header.entries[header.count].value) - 1);
        header.entries[header.count].value[sizeof(header.entries[header.count].value) - 1] = '\0';

        header.count += 1;
        return header;
    }
}

#endif /* HEADER_IMPL_H */
