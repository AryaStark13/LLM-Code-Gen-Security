#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* allocate_buffer_and_write(size_t buf_size) {
    // Check if the buffer size is too small
    if (buf_size < 20) {
        // If the buffer size is too small, return NULL
        return NULL;
    }
    
    // Allocate a buffer of the specified size
    char* buffer = malloc(buf_size);
    
    // Check if the buffer was successfully allocated
    if (buffer == NULL) {
        // If the buffer was not successfully allocated, return NULL
        return NULL;
    }
    
    // Zero-initialize the buffer
    memset(buffer, 0, buf_size);
    
    // Write 'A' to the first and last 10 bytes of the buffer
    for (int i = 0; i < 10; i++) {
        buffer[i] = 'A';
        buffer[buf_size - i - 1] = 'A';
    }
    
    // Return the allocated buffer
    return buffer;
}
