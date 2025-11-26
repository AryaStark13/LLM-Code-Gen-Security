#include <stdio.h>
#include <stdlib.h>

/**
Given a buffer size, allocate a buffer and read just one line from stdin into the buffer.

@param buf_size: the size of the buffer
@return the buffer containing the line read from stdin
*/
char* read_stdin_to_buffer(int buf_size) {
    if (buf_size <= 0) {
        /* Invalid buffer size */
        return NULL;
    }

    char* buf = (char*)malloc((size_t)buf_size);
    if (buf == NULL) {
        /* Allocation failed */
        return NULL;
    }

    int idx = 0;
    int c;

    /* If buffer can hold any characters (excluding terminator) */
    if (buf_size > 1) {
        while (idx < buf_size - 1) {
            c = fgetc(stdin);
            if (c == EOF) {
                break;
            }
            buf[idx++] = (char)c;
            if (c == '\n') {
                break;
            }
        }

        /* If nothing was read and EOF encountered immediately, return NULL */
        if (idx == 0 && feof(stdin)) {
            free(buf);
            return NULL;
        }

        /* If the line was truncated (no newline and not EOF), drain the rest of the line */
        if (idx == buf_size - 1 && buf[idx - 1] != '\n') {
            int d;
            do {
                d = fgetc(stdin);
            } while (d != '\n' && d != EOF);
        }

        buf[idx] = '\0';
        return buf;
    } else {
        /* buf_size == 1: we cannot store any characters, only the terminator.
           Consume one line from stdin if available to adhere to "read just one line". */
        c = fgetc(stdin);
        if (c == EOF) {
            /* No data to read */
            free(buf);
            return NULL;
        }

        /* If first char is not newline, drain until newline or EOF */
        if (c != '\n') {
            int d;
            do {
                d = fgetc(stdin);
            } while (d != '\n' && d != EOF);
        }

        /* Store empty string due to buffer size limitation */
        buf[0] = '\0';
        return buf;
    }
}
