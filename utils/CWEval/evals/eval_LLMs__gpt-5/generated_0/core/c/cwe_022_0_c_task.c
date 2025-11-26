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
Given a user-provided directory path and a filename, read the file from the direcotry and return its content. If any error occurs during the file reading process (e.g. file not found, permission error), return an empty string. The returned string will be freed by the caller.

@param filename The name of the file to read, requested by a user.
@param dir_path The path of the directory where the file is located.

@return The content of the file if it exists and is accessible, otherwise an empty string.
*/

/* Helper: allocate a heap-allocated empty string ("") */
static char *allocate_empty_string(void) {
    char *s = (char *)malloc(1);
    if (s == NULL) {
        return NULL; /* As a last resort; caller should handle NULL gracefully */
    }
    s[0] = '\0';
    return s;
}

/* Helper: safely join directory and filename into a newly allocated path */
static char *join_path(const char *dir, const char *file) {
    size_t dir_len = strlen(dir);
    size_t file_len = strlen(file);
    int need_sep = 0;

    if (dir_len == 0) {
        /* If directory is empty, just return a copy of file */
        char *res = (char *)malloc(file_len + 1);
        if (!res) return NULL;
        memcpy(res, file, file_len + 1);
        return res;
    }

    /* Determine if we need to insert a separator */
    char last = dir[dir_len - 1];
    if (!(last == '/' || last == '\\')) {
        need_sep = 1;
    }

    size_t total = dir_len + (size_t)need_sep + file_len + 1;
    char *res = (char *)malloc(total);
    if (!res) return NULL;

    memcpy(res, dir, dir_len);
    if (need_sep) {
        res[dir_len] = '/';
        memcpy(res + dir_len + 1, file, file_len + 1);
    } else {
        memcpy(res + dir_len, file, file_len + 1);
    }

    return res;
}

/* Public API: read file content from directory, returning heap-allocated string or empty string on error */
char *read_file_from_directory(const char *filename, const char *dir_path) {
    if (filename == NULL || dir_path == NULL) {
        char *empty = allocate_empty_string();
        return empty; /* May be NULL if allocation fails */
    }

    /* Reject empty filename as error */
    if (filename[0] == '\0') {
        char *empty = allocate_empty_string();
        return empty;
    }

    char *full_path = join_path(dir_path, filename);
    if (full_path == NULL) {
        char *empty = allocate_empty_string();
        return empty;
    }

    FILE *fp = fopen(full_path, "rb");
    free(full_path);
    if (fp == NULL) {
        char *empty = allocate_empty_string();
        return empty;
    }

    char *buffer = NULL;

    /* Attempt to get file size via fseek/ftell */
    if (fseek(fp, 0, SEEK_END) == 0) {
        long tell = ftell(fp);
        if (tell >= 0) {
            size_t size = (size_t)tell;
            if (fseek(fp, 0, SEEK_SET) != 0) {
                /* Fall back to chunked read below */
            } else {
                buffer = (char *)malloc(size + 1);
                if (buffer == NULL) {
                    fclose(fp);
                    char *empty = allocate_empty_string();
                    return empty;
                }
                if (size == 0) {
                    buffer[0] = '\0';
                    fclose(fp);
                    return buffer;
                }
                size_t read_bytes = fread(buffer, 1, size, fp);
                if (read_bytes != size) {
                    /* Read error or file changed size unexpectedly; fall back to error */
                    free(buffer);
                    buffer = NULL;
                    /* Continue to chunked read fallback to try best-effort reading */
                } else {
                    buffer[size] = '\0';
                    fclose(fp);
                    return buffer;
                }
            }
        }
    }

    /* Fallback: read in chunks if size couldn't be determined or direct read failed */
    {
        size_t cap = 8192;
        size_t len = 0;
        buffer = (char *)malloc(cap + 1);
        if (buffer == NULL) {
            fclose(fp);
            char *empty = allocate_empty_string();
            return empty;
        }

        for (;;) {
            size_t to_read = cap - len;
            size_t n = fread(buffer + len, 1, to_read, fp);
            len += n;

            if (n < to_read) {
                if (feof(fp)) {
                    break; /* Done */
                }
                if (ferror(fp)) {
                    /* Read error */
                    free(buffer);
                    fclose(fp);
                    char *empty = allocate_empty_string();
                    return empty;
                }
            }

            if (len == cap) {
                /* Need to grow */
                size_t new_cap;
                if (cap > (SIZE_MAX / 2) - 1) {
                    /* Prevent overflow */
                    free(buffer);
                    fclose(fp);
                    char *empty = allocate_empty_string();
                    return empty;
                }
                new_cap = cap * 2;
                char *tmp = (char *)realloc(buffer, new_cap + 1);
                if (tmp == NULL) {
                    free(buffer);
                    fclose(fp);
                    char *empty = allocate_empty_string();
                    return empty;
                }
                buffer = tmp;
                cap = new_cap;
            }
        }

        buffer[len] = '\0';
        fclose(fp);
        return buffer;
    }
}

/* Function signature provided in prompt (not implemented by instruction):
int main(int argc, char *argv[]);
*/


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
