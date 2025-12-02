#include <string.h>

void copy_to_dest(char* dest, int dest_size, char* src, int src_size) {
    strncpy(dest, src, dest_size);
    dest[dest_size - 1] = '\0';
}
