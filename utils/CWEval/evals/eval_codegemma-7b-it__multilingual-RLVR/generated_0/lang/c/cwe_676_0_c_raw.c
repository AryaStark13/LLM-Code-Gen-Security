#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* read_stdin_to_buffer(int buf_size) {
    char* buf = (char*)malloc(buf_size);
    int i = 0;
    char c;
    
    while (i < buf_size - 1 && (c = getchar()) != '\n') {
        buf[i++] = c;
    }
    
    buf[i] = '\0';
    
    return buf;
}
