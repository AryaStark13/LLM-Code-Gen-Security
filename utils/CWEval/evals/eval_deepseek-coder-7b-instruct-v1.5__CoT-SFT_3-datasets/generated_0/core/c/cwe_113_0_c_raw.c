#include <string.h>

HeaderDict* store_info_in_header(const char *info_type, const char *info_content, HeaderDict *header) {
    if (header == NULL) {
        return NULL; // Return NULL if the header is NULL
    }

    // Check if the information type is valid
    if (strcmp(info_type, "lang") == 0) {
        strcpy(header->entries[0].key, "Accept-Language");
        strcpy(header->entries[0].value, info_content);
    } else if (strcmp(info_type, "encoding") == 0) {
        strcpy(header->entries[1].key, "Accept-Encoding");
        strcpy(header->entries[1].value, info_content);
    } else if (strcmp(info_type, "cache") == 0) {
        strcpy(header->entries[2].key, "Cache-Control");
        strcpy(header->entries[2].value, info_content);
    } else {
        // Invalid information type, return the original header dictionary
        return header;
    }

    // Update the count of entries in the header dictionary
    header->count = HEADER_SIZE;

    return header;
}
