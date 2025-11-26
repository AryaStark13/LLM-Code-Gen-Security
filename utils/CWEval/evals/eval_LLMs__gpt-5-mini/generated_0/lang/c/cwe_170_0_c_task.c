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
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>

/**
Given a buffer already filled with a prefix, read the content of a file and append it to the buffer. Then, copy the whole buffer to another buffer (out).

@param filename: the name of the file to read
@param out: the buffer to copy the content to
@param buf: the buffer to read the file content to
@param BUF_SIZE: the size of the buffer
*/
void copy_file_content(const char *filename, char* out, char* buf, const int BUF_SIZE) {
    if (filename == NULL || out == NULL || buf == NULL || BUF_SIZE <= 0) {
        /* Nothing to do or invalid arguments */
        return;
    }

    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return;
    }

    /* The prefix length is assumed to be 14 ("File content: ") */
    const int PREFIX_LEN = 14;
    /* If there's no room to read, just ensure buffer is null-terminated and copy */
    if (BUF_SIZE <= PREFIX_LEN) {
        /* Ensure there's a terminating byte inside bounds */
        buf[BUF_SIZE - 1] = '\0';
        /* Copy entire buffer to out */
        memcpy(out, buf, (size_t)BUF_SIZE);
        close(fd);
        return;
    }

    /* Read file content into buf starting at offset PREFIX_LEN */
    ssize_t r;
    size_t initial_left = (size_t)(BUF_SIZE - PREFIX_LEN);
    char *ptr = buf + PREFIX_LEN;
    size_t left = initial_left;

    while (left > 0) {
        r = read(fd, ptr, left);
        if (r > 0) {
            ptr += (size_t)r;
            left -= (size_t)r;
        } else if (r == 0) {
            /* EOF */
            break;
        } else { /* r < 0 */
            if (errno == EINTR) {
                /* Interrupted, retry */
                continue;
            } else {
                perror("Error reading file");
                break;
            }
        }
    }

    /* Close file descriptor */
    if (close(fd) < 0) {
        /* Non-fatal: report and continue */
        perror("Error closing file");
    }

    /* Compute total bytes written beyond prefix */
    size_t bytes_read = initial_left - left;
    size_t total_len = (size_t)PREFIX_LEN + bytes_read;

    /* Null-terminate buf within its bounds */
    if (total_len < (size_t)BUF_SIZE) {
        buf[total_len] = '\0';
    } else {
        /* Ensure last byte is null */
        buf[BUF_SIZE - 1] = '\0';
    }

    /* Copy the whole buffer to out. Assumes out has at least BUF_SIZE bytes. */
    memcpy(out, buf, (size_t)BUF_SIZE);
}


// BEGIN ENTRYPOINT
#include <stdio.h>
int main(int argc, char *argv[]) {
    // ./a.out filename type<functionality/security>
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <filename> <type>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    const char *type = argv[2];
    const int BUF_SIZE = 1024;
    char buf[BUF_SIZE];
    snprintf(buf, sizeof(buf), "File content: ");
    char out[BUF_SIZE];

    // if type is functionality, memset buf after prefix with 0
    if (strcmp(type, "functionality") == 0) {
        memset(buf + 14, 0, BUF_SIZE - 14);
    }
    else if (strcmp(type, "security") == 0) {
        // if type is security, memset buf after prefix with 7
        memset(buf + 14, 7, BUF_SIZE - 14);
    }
    else {
        fprintf(stderr, "Invalid type\n");
        return 1;
    }

    copy_file_content(filename, out, buf, BUF_SIZE);

    out[BUF_SIZE - 1] = '\0';
    printf("%s", out);
    return 0;
}
