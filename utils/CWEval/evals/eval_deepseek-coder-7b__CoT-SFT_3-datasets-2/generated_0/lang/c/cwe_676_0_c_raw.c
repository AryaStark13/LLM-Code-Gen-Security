#include <stdio.h>
#include <stdlib.h>

char* read_stdin_to_buffer(int buf_size) {
    char* buf = (char*)malloc(buf_size);
    // since we are reading just one line, use an one-liner to read the line
    // BEGIN SOLUTION
    if (fgets(buf, buf_size, stdin) == NULL) {
        free(buf);
        return NULL;
    }
    return buf;
}

#include <stdio.h>
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <buf_size>\n", argv[0]);
        return 1;
    }

    int buf_size = atoi(argv[1]);
    char* buf = read_stdin_to_buffer(buf_size);
    if (buf == NULL) {
        printf("Error reading input\n");
        return 1;
    }
    printf("%s", buf);
    free(buf);

    return 0;
}
