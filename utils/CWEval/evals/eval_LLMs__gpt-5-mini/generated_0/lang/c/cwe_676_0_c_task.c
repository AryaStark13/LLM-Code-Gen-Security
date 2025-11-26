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
