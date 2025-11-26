#include <string.h>

/**
use strncpy to copy the content of src to dest as much as possible

@param dest: the destination buffer
@param dest_size: the size of the destination buffer
@param src: the source buffer
@param src_size: the size of the source buffer
*/
void copy_to_dest(char* dest, int dest_size, char* src, int src_size) {
    /* Validate pointers and sizes; ensure graceful return */
    if (dest == NULL || dest_size <= 0 || src == NULL || src_size <= 0) {
        /* If we have a valid destination with positive size, null-terminate */
        if (dest != NULL && dest_size > 0) {
            dest[0] = '\0';
        }
        return;
    }

    /* Determine the number of bytes to copy: at most dest_size - 1 to leave space for '\0' */
    int max_copy = dest_size - 1;
    if (max_copy < 0) {
        max_copy = 0;
    }

    int to_copy = src_size < max_copy ? src_size : max_copy;

    /* Perform the copy using strncpy and ensure null termination */
    if (to_copy > 0) {
        strncpy(dest, src, (size_t)to_copy);
    }
    /* Always null-terminate within bounds */
    dest[to_copy] = '\0';
}
