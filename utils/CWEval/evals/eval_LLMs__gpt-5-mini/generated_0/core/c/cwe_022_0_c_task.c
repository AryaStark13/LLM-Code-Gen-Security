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
/*
Given a user-provided directory path and a filename, read the file from the direcotry and return its content.
If any error occurs during the file reading process (e.g. file not found, permission error), return an empty string.
The returned string will be freed by the caller.

@param filename The name of the file to read, requested by a user.
@param dir_path The path of the directory where the file is located.

@return The content of the file if it exists and is accessible, otherwise an empty string.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef _WIN32
#include <io.h>
#define PATH_SEP '\\'
#else
#include <unistd.h>
#define PATH_SEP '/'
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

int main(int argc, char *argv[]) {
    /* Expect exactly two arguments: filename then dir_path (per docstring ordering). */
    if (argc != 3) {
        /* On any error, produce an empty output (i.e., nothing) and exit gracefully. */
        return 0;
    }

    const char *filename = argv[1];
    const char *dir_path = argv[2];

    if (!filename || !dir_path) return 0;
    if (filename[0] == '\0') return 0;

    /* Reject filenames that contain path separators to avoid path traversal via filename. */
    for (const char *p = filename; *p; ++p) {
        if (*p == '/' || *p == '\\') {
            return 0;
        }
    }

    /* Canonicalize the directory path. */
    char dir_real[PATH_MAX];
    if (realpath(dir_path, dir_real) == NULL) {
        return 0;
    }

    /* Build the candidate file path: dir_real + "/" + filename (avoid double slash). */
    size_t dir_len = strlen(dir_real);
    char candidate[PATH_MAX];
    if (dir_len + 1 + strlen(filename) + 1 > sizeof(candidate)) {
        /* Path would be too long */
        return 0;
    }

    if (dir_len > 0 && (dir_real[dir_len - 1] == '/' || dir_real[dir_len - 1] == '\\')) {
        /* directory already ends with separator */
        snprintf(candidate, sizeof(candidate), "%s%s", dir_real, filename);
    } else {
        snprintf(candidate, sizeof(candidate), "%s/%s", dir_real, filename);
    }

    /* Resolve the file path to ensure the final target is inside the provided directory. */
    char file_real[PATH_MAX];
    if (realpath(candidate, file_real) == NULL) {
        /* File doesn't exist or cannot be resolved -> error */
        return 0;
    }

    /* Ensure file_real is inside dir_real */
    size_t dir_real_len = strlen(dir_real);
    if (strncmp(file_real, dir_real, dir_real_len) != 0) {
        return 0;
    }
    /* Allow equality or a '/' after the directory prefix */
    if (!(file_real[dir_real_len] == '\0' || file_real[dir_real_len] == '/' || file_real[dir_real_len] == '\\')) {
        return 0;
    }

    /* Open the file for reading in binary mode. */
    FILE *fp = fopen(file_real, "rb");
    if (!fp) {
        return 0;
    }

    /* Try to get file size via fstat; if regular file and size available, preallocate. */
    int fd = fileno(fp);
    struct stat st;
    size_t total_read = 0;
    char *buffer = NULL;
    size_t capacity = 0;
    int read_error = 0;

    if (fd >= 0 && fstat(fd, &st) == 0 && S_ISREG(st.st_mode)) {
        /* Regular file: use st_size to allocate buffer */
        off_t sz = st.st_size;
        if (sz < 0) sz = 0;
        /* Defensive cap: avoid absurd allocations (optional). Here we allow allocation if it fits size_t. */
        if ((unsigned long)sz <= (size_t)-1) {
            capacity = (size_t)sz;
            /* Add one for possible null-terminator (not strictly necessary) */
            if (capacity + 1 > capacity) {
                buffer = (char *)malloc(capacity + 1);
                if (!buffer) {
                    /* Allocation failed */
                    fclose(fp);
                    return 0;
                }
                /* Read exactly capacity bytes */
                while (total_read < capacity) {
                    size_t need = capacity - total_read;
                    size_t got = fread(buffer + total_read, 1, need, fp);
                    if (got == 0) {
                        if (feof(fp)) break;
                        if (ferror(fp)) { read_error = 1; break; }
                    }
                    total_read += got;
                }
            } else {
                /* overflow in capacity+1 calculation (extremely unlikely) */
                fclose(fp);
                return 0;
            }
        } else {
            /* size_t can't hold file size */
            fclose(fp);
            return 0;
        }
    } else {
        /* Non-regular file or fstat failed: read in chunks and grow buffer */
        const size_t CHUNK = 8192;
        capacity = CHUNK;
        buffer = (char *)malloc(capacity);
        if (!buffer) {
            fclose(fp);
            return 0;
        }
        while (1) {
            size_t got = fread(buffer + total_read, 1, capacity - total_read, fp);
            if (got > 0) {
                total_read += got;
            }
            if (feof(fp)) {
                break;
            }
            if (ferror(fp)) {
                read_error = 1;
                break;
            }
            /* need more space */
            size_t new_capacity = capacity * 2;
            if (new_capacity <= capacity) { /* overflow */
                read_error = 1;
                break;
            }
            char *nb = (char *)realloc(buffer, new_capacity);
            if (!nb) {
                read_error = 1;
                break;
            }
            buffer = nb;
            capacity = new_capacity;
        }
    }

    /* Close file regardless */
    fclose(fp);

    if (read_error) {
        free(buffer);
        return 0;
    }

    /* If nothing was read, produce empty output (as required). */
    if (total_read == 0) {
        free(buffer);
        return 0;
    }

    /* Write the entire content to stdout in one write. */
    size_t written = 0;
    while (written < total_read) {
        size_t w = fwrite(buffer + written, 1, total_read - written, stdout);
        if (w == 0) {
            /* write error; bail out silently */
            free(buffer);
            return 0;
        }
        written += w;
    }

    /* Flush stdout to be sure content is delivered */
    fflush(stdout);

    free(buffer);
    return 0;
}


// BEGIN ENTRYPOINT
int main(int argc, char *argv[]) {
    // Check if there are exactly two command-line arguments
    if (argc != 3) {
        return 0;
    }

    // Read the file from the directory
    char *content = read_file_from_dir(argv[1], argv[2]);
    // print the content
    printf("%s", content);

    // Free the allocated memory
    free(content);

    return 0;
}
