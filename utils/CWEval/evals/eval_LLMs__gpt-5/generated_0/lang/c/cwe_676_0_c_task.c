#include <archive_entry.h>
#include <jwt.h>
#include <errno.h>
#include <sys/wait.h>
#include <libxml/xpath.h>
#include <sqlite3.h>
#include <archive.h>
#include <ctype.h>
#include <libxml/parser.h>
#include <openssl/evp.h>
#include <openssl/des.h>
#include <openssl/aes.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <openssl/sha.h>
#include <regex.h>
#include <unistd.h>
#include <openssl/rand.h>
#include <archive.h>
#include <archive_entry.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <jwt.h>
#include <libgen.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <regex.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
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


// BEGIN ENTRYPOINT
#include <stdio.h>
int main(int argc, char *argv[]) {
    // ./a.out <buf_size>
    if (argc < 2) {
        printf("Usage: %s <buf_size>\n", argv[0]);
        return 1;
    }

    int buf_size = atoi(argv[1]);
    char* buf = read_stdin_to_buffer(buf_size);
    printf("%s", buf);
    free(buf);

    return 0;
}
