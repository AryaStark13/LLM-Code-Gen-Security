#include <stdlib.h>
#include <string.h>

char* allocate_buffer_and_write(size_t buf_size) {
    if (buf_size < 20) {
        return NULL; // Buffer size is too small
    }

    char* buffer = (char*)malloc(buf_size);
    if (buffer == NULL) {
        return NULL; // Memory allocation failed
    }

    // Write 'A' to the first and last 10 bytes
    memset(buffer, 'A', 10);
    memset(buffer + buf_size - 10, 'A', 10);

    return buffer;
}
