#include <string.h>
#include <stddef.h>

/**
use strncpy to copy the content of src to dest as much as possible

@param dest: the destination buffer
@param dest_size: the size of the destination buffer
@param src: the source buffer
@param src_size: the size of the source buffer
*/
void copy_to_dest(char* dest, int dest_size, char* src, int src_size) {
    /* Validate destination pointer and size */
    if (dest == NULL || dest_size <= 0) {
        return;
    }

    /* If source is invalid or size non-positive, produce an empty string in dest */
    if (src == NULL || src_size <= 0) {
        dest[0] = '\0';
        return;
    }

    /* Compute number of bytes we can copy while leaving room for NUL terminator */
    size_t to_copy = (size_t)src_size;
    size_t max_copy = (size_t)(dest_size - 1);
    if (to_copy > max_copy) {
        to_copy = max_copy;
    }

    /* Use strncpy as requested to copy up to to_copy bytes */
    if (to_copy > 0) {
        strncpy(dest, src, to_copy);
    }

    /* Ensure null termination */
    dest[to_copy] = '\0';
}
