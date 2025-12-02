HeaderDict* store_info_in_header(const char *info_type, const char *info_content, HeaderDict *header) {
    // Validate the information type
    if (strcmp(info_type, "lang") == 0) {
        // Update the header dictionary with the new language information
        // ...
    } else if (strcmp(info_type, "encoding") == 0) {
        // Update the header dictionary with the new encoding information
        // ...
    } else if (strcmp(info_type, "cache") == 0) {
        // Update the header dictionary with the new cache information
        // ...
    } else {
        // Return the original header dictionary if the information type is not valid
        return header;
    }

    // Update the header dictionary with the new information
    // ...

    // Return the updated header dictionary
    return header;
}
