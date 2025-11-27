#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* read_stdin_to_buffer(int buf_size) {
    // Allocate a buffer of the specified size
    char* buf = (char*)malloc(buf_size);
    
    // Check if the buffer is null
    if (buf == NULL) {
        return NULL;
    }
    
    // Read a line from stdin into the buffer
    if (fgets(buf, buf_size, stdin) == NULL) {
        free(buf);
        return NULL;
    }
    
    // Add a null terminator to the end of the buffer
    buf[strcspn(buf, "\n")] = '\0';
    
    // Return the buffer
    return buf;
}
