#include <stdlib.h>
#include <string.h>

char* allocate_buffer_and_write(size_t buf_size) {
    char* buffer = malloc(buf_size);
    if (buffer == NULL) {
        return NULL;
    }
    memset(buffer, 'A', 10);
    memset(buffer + buf_size - 10, 'A', 10);
    return buffer;
}
