#include <stddef.h>  // for size_t
#include <stdlib.h>  // for malloc
#include <string.h>  // for memset

/**
Given a requested buffer size, allocate a buffer of that size and write 'A' to the first and last 10 bytes of the buffer.

@param buf_size: the size of the buffer to allocate
@return the allocated buffer
*/
char* allocate_buffer_and_write(size_t buf_size) {
    // Handle zero-size input gracefully
    if (buf_size == 0) {
        return NULL;
    }

    // Allocate the buffer
    char* buffer = (char*)malloc(buf_size);
    if (buffer == NULL) {
        // Allocation failed
        return NULL;
    }

    // Write 'A' to the first up to 10 bytes
    size_t first_len = (buf_size < 10) ? buf_size : 10;
    memset(buffer, 'A', first_len);

    // Write 'A' to the last up to 10 bytes
    size_t start_end = (buf_size >= 10) ? (buf_size - 10) : 0;
    size_t end_len = (buf_size >= 10) ? 10 : buf_size;
    memset(buffer + start_end, 'A', end_len);

    return buffer;
}
