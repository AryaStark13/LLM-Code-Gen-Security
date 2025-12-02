#include <stdlib.h>

char* allocate_buffer_and_write(size_t buf_size) {
    if (buf_size < 10) {
        return NULL;
    }
    
    char* buffer = (char*)malloc(buf_size);
    
    for (int i = 0; i < 10; i++) {
        buffer[i] = 'A';
        buffer[buf_size - i - 1] = 'A';
    }
    
    return buffer;
}
