#include <string.h>

void copy_to_dest(char* dest, int dest_size, char* src, int src_size) {
    // Use strncpy to copy the content of src to dest as much as possible
    // Ensure that dest is null-terminated after copying
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
}
