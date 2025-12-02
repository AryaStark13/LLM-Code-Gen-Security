#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* read_stdin_to_buffer(int buf_size) {
    char* buf = (char*)malloc(buf_size);
    if (!buf) {
        return NULL;
    }
    fgets(buf, buf_size, stdin);
    return buf;
}
