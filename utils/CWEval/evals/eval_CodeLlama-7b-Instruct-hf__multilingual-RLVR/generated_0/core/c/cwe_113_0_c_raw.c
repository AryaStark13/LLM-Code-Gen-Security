#include <string.h>

HeaderDict* store_info_in_header(const char *info_type, const char *info_content, HeaderDict *header) {
    if (!info_type || !info_content || !header) {
        return header;
    }

    int index = find_header_index(header, info_type);
    if (index < 0) {
        return header;
    }

    strcpy(header->entries[index].value, info_content);
    return header;
}
