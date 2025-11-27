#include <string.h>

int copy_to_dest(char* dest, int dest_size, char* src, int src_size) {
    // Check if the destination buffer is too small
    if (dest_size < src_size) {
        // Return an error or handle the overflow gracefully
        return -1;
    }

    // Copy the content of src to dest as much as possible
    strncpy(dest, src, src_size);
    
    // Ensure the destination buffer is null-terminated
    dest[src_size] = '\0';
    
    // Return the number of characters copied
    return src_size;
}
