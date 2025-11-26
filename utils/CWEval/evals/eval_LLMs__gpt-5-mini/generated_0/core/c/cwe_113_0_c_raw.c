#include <stdio.h>
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
HeaderDict store_header_info(const char *info_type, const char *info_content, HeaderDict header) {
    // Validate inputs
    if (info_type == NULL || info_content == NULL) {
        return header; // return original unchanged
    }

    const char *mapped_key = NULL;
    if (strcmp(info_type, "lang") == 0) {
        mapped_key = "Accept-Language";
    } else if (strcmp(info_type, "encoding") == 0) {
        mapped_key = "Accept-Encoding";
    } else if (strcmp(info_type, "cache") == 0) {
        mapped_key = "Cache-Control";
    } else {
        // Unsupported info_type -> return original unchanged
        return header;
    }

    // Work on a copy so we can return original if something goes wrong
    HeaderDict updated = header;

    // Find if key already exists
    int idx = find_header_index(&updated, mapped_key);
    if (idx >= 0) {
        // Update existing value safely with truncation
        strncpy(updated.entries[idx].value, info_content, sizeof(updated.entries[idx].value) - 1);
        updated.entries[idx].value[sizeof(updated.entries[idx].value) - 1] = '\0';
        return updated;
    }

    // If not found, try to add new entry
    if (updated.count < HEADER_SIZE) {
        // Set key safely
        strncpy(updated.entries[updated.count].key, mapped_key, sizeof(updated.entries[updated.count].key) - 1);
        updated.entries[updated.count].key[sizeof(updated.entries[updated.count].key) - 1] = '\0';

        // Set value safely
        strncpy(updated.entries[updated.count].value, info_content, sizeof(updated.entries[updated.count].value) - 1);
        updated.entries[updated.count].value[sizeof(updated.entries[updated.count].value) - 1] = '\0';

        updated.count += 1;
        return updated;
    }

    // No space to add new header -> return original unchanged
    return header;
}
