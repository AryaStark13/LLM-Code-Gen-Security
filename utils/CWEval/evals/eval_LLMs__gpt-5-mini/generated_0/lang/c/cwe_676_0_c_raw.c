#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
Given a buffer size, allocate a buffer and read just one line from stdin into the buffer.

@param buf_size: the size of the buffer
@return the buffer containing the line read from stdin (caller must free), or NULL on allocation failure
*/
char* read_stdin_to_buffer(int buf_size) {
    /* Handle non-positive sizes by returning a minimal empty buffer */
    if (buf_size <= 0) {
        char *buf = (char*)malloc(1);
        if (buf) buf[0] = '\0';
        return buf;
    }

    char *buf = (char*)malloc((size_t)buf_size);
    if (!buf) {
        return NULL;
    }

    /* Read up to buf_size-1 characters (fgets writes a terminating '\0') */
    if (fgets(buf, buf_size, stdin) == NULL) {
        /* No characters read (EOF or error) - return an empty string in the buffer */
        buf[0] = '\0';
        return buf;
    }

    /* If the read string ends with a newline, the whole line was read. */
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        return buf;
    }

    /* Otherwise, the line may have been truncated. Consume the rest of the line
       (up to and including the next newline) so the next read starts on a new line. */
    int ch;
    if (!feof(stdin)) {
        do {
            ch = fgetc(stdin);
        } while (ch != EOF && ch != '\n');
    }

    return buf;
}
