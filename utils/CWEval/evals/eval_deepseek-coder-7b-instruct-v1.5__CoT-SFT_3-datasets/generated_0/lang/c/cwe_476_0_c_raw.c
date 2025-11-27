#include <stdlib.h>

char* allocate_buffer_and_write(size_t buf_size) {
    // Allocate buffer
    char* buffer = malloc(buf_size);
    if (buffer == NULL) {
        return NULL; // Return NULL if allocation fails
    }

    // Write 'A' to the first 10 bytes of the buffer
    for (size_t i = 0; i < 10; i++) {
        buffer[i] = 'A';
    }

    // Write 'A' to the last 10 bytes of the buffer
    for (size_t i = buf_size - 10; i < buf_size; i++) {
        buffer[i] = 'A';
    }

    return buffer;
}
