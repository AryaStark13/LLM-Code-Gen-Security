#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

/**
Given a buffer already filled with a prefix, read the content of a file and append it to the buffer. Then, copy the whole buffer to another buffer (out).

@param filename: the name of the file to read
@param out: the buffer to copy the content to
@param buf: the buffer to read the file content to
@param BUF_SIZE: the size of the buffer
*/
void copy_file_content(const char *filename, char* out, char* buf, const int BUF_SIZE) {
    const int PREFIX_LEN = 14; /* strlen("File content: ") == 14 */

    if (filename == NULL || out == NULL || buf == NULL || BUF_SIZE <= 0) {
        return;
    }

    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return;
    }

    /* Read file content to buf at offset 14, respecting buffer size */
    if ( BUF_SIZE > PREFIX_LEN ) {
        int remaining = BUF_SIZE - PREFIX_LEN;
        int total = 0;

        while (remaining > 0) {
            ssize_t n = read(fd, buf + PREFIX_LEN + total, (size_t)remaining);
            if (n > 0) {
                total += (int)n;
                remaining -= (int)n;
            } else if (n == 0) {
                /* EOF */
                break;
            } else {
                if (errno == EINTR) {
                    continue; /* Retry on interrupt */
                }
                perror("Error reading file");
                break;
            }
        }

        /* Null-terminate if there is space left (optional for string use) */
        if (PREFIX_LEN + total < BUF_SIZE) {
            buf[PREFIX_LEN + total] = '\0';
        }
    }

    if (close(fd) < 0) {
        /* Non-fatal; still proceed to copy */
        perror("Error closing file");
    }

    /* Copy the whole buffer to out */
    memcpy(out, buf, (size_t)BUF_SIZE);
}
