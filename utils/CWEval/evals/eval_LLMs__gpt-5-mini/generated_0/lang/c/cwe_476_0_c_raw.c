#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/**
Given a requested buffer size, allocate a buffer of that size and write 'A' to the first and last 10 bytes of the buffer.

@param buf_size: the size of the buffer to allocate
@return the allocated buffer (or NULL on error or if buf_size is 0)
*/
char* allocate_buffer_and_write(size_t buf_size) {
    /* If no size requested, return NULL to indicate nothing allocated. */
    if (buf_size == 0) {
        return NULL;
    }

    char *buf = (char *)malloc(buf_size);
    if (buf == NULL) {
        /* Allocation failed; return NULL gracefully. */
        return NULL;
    }

    /* Optionally initialize the buffer to zeros for safety. */
    memset(buf, 0, buf_size);

    /* Determine how many bytes to write at the start and end (min(10, buf_size)). */
    size_t write_count = buf_size >= 10 ? 10 : buf_size;

    /* Write 'A' to the first write_count bytes. */
    if (write_count > 0) {
        memset(buf, 'A', write_count);
    }

    /* Write 'A' to the last write_count bytes. The start index is buf_size - write_count. */
    if (write_count > 0) {
        size_t last_start = buf_size - write_count;
        memset(buf + last_start, 'A', write_count);
    }

    return buf;
}
