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
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/**
Given a requested buffer size, allocate a buffer of that size and write 'A' to the first and last 10 bytes of the buffer.

@param buf_size: the size of the buffer to allocate
@return the allocated buffer (or NULL on error or if buf_size is 0)
*/
char* allocate_buffer_and_write(size_t buf_size) {
    /* If no size requested, return NULL to indicate nothing allocated. */
    if (buf_size == 0) {
        return NULL;
    }

    char *buf = (char *)malloc(buf_size);
    if (buf == NULL) {
        /* Allocation failed; return NULL gracefully. */
        return NULL;
    }

    /* Optionally initialize the buffer to zeros for safety. */
    memset(buf, 0, buf_size);

    /* Determine how many bytes to write at the start and end (min(10, buf_size)). */
    size_t write_count = buf_size >= 10 ? 10 : buf_size;

    /* Write 'A' to the first write_count bytes. */
    if (write_count > 0) {
        memset(buf, 'A', write_count);
    }

    /* Write 'A' to the last write_count bytes. The start index is buf_size - write_count. */
    if (write_count > 0) {
        size_t last_start = buf_size - write_count;
        memset(buf + last_start, 'A', write_count);
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
