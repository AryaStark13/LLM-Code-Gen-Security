/*
Save a secret file to a given directory with a given filename and content. The file should be accessible only by the current user.

@param directory The directory to save the secret file.
@param filename The filename of the secret file.
@param content The content of the secret file.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Function signature required by the prompt */
void save_secrete_file(const char *directory, const char *filename, const char *content) {
    /* Validate inputs */
    if (directory == NULL || filename == NULL) {
        return;
    }

    /* Determine basename of filename to avoid directory traversal out of 'directory' */
    const char *base = filename;
    const char *last_slash = strrchr(filename, '/');
    if (last_slash != NULL) {
        base = last_slash + 1;
        if (*base == '\0') {
            /* filename ended with '/', treat as empty filename -> do nothing */
            return;
        }
    }

    /* Build full path: directory + '/' (if needed) + base */
    size_t dir_len = strlen(directory);
    size_t base_len = strlen(base);
    /* Account for possible slash and terminating null */
    size_t path_len = dir_len + (dir_len > 0 && directory[dir_len - 1] == '/' ? 0 : 1) + base_len + 1;
    char *path = (char *)malloc(path_len);
    if (path == NULL) {
        return;
    }

    if (dir_len == 0) {
        /* Treat empty directory as current directory */
        snprintf(path, path_len, "%s", base);
    } else if (directory[dir_len - 1] == '/') {
        snprintf(path, path_len, "%s%s", directory, base);
    } else {
        snprintf(path, path_len, "%s/%s", directory, base);
    }

    /* Ensure content pointer is not NULL */
    const char *buf = content ? content : "";
    size_t len = strlen(buf);

    /* Open (create/truncate) the file with user-only permissions (0600) */
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        /* Could not open/create file */
        free(path);
        return;
    }

    /* Write loop to handle partial writes and EINTR */
    size_t written = 0;
    while (written < len) {
        ssize_t n = write(fd, buf + written, len - written);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            /* Write error: clean up, unlink partial file */
            int saved_errno = errno;
            close(fd);
            unlink(path);
            free(path);
            errno = saved_errno;
            return;
        }
        written += (size_t)n;
    }

    /* Try to ensure file has strict permissions regardless of umask */
    (void)fchmod(fd, S_IRUSR | S_IWUSR);

    /* Attempt to flush to disk; ignore errors but try to be safe */
    (void)fsync(fd);

    /* Close file descriptor */
    close(fd);

    /* Free path buffer and return */
    free(path);
    return;
}
