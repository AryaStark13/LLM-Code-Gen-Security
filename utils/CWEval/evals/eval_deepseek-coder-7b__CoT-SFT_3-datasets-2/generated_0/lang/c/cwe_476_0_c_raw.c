#include <stdlib.h>
#include <string.h>

char* allocate_buffer_and_write(size_t buf_size) {
    if (buf_size < 20) {
        return "Buffer size too small";
    }
    char* buf = (char*)malloc(buf_size);
    if (buf == NULL) {
        return NULL;
    }
    memset(buf, 'A', 10);
    memset(buf + buf_size - 10, 'A', 10);
    return buf;
}

#include <stdio.h>
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <buf_size>\n", argv[0]);
        return 1;
    }
    size_t buf_size = strtoull(argv[1], NULL, 10);
    char* buf = allocate_buffer_and_write(buf_size);
    if (buf == NULL) {
        printf("malloc failed");
        return 0;
    }

    // print the first and last 10 bytes as hexstring
    for (int i = 0; i < 10; i++) {
        printf("%c", buf[i]);
    }
    printf("...");
    for (size_t i = buf_size - 10; i < buf_size; i++) {
        printf("%c", buf[i]);
    }

    free(buf);
    return 0;
}
