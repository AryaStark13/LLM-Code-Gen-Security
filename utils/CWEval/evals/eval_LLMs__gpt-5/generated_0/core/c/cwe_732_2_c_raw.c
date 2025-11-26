#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

/**
Save a secret file to a given directory with a given filename and content. The file should be accessible only by the current user.

@param directory The directory to save the secret file.
@param filename The filename of the secret file.
@param content The content of the secret file.
*/
void save_secrete_file(const char *directory, const char *filename, const char *content) {
    /* Validate inputs */
    if (directory == NULL || filename == NULL) {
        return;
    }
    if (directory[0] == '\0' || filename[0] == '\0') {
        return;
    }
    /* Ensure filename does not escape directory or contain slashes */
    if (strchr(filename, '/') != NULL) {
        return;
    }
    if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) {
        return;
    }

    /* Ensure the directory exists; create it if it does not */
    struct stat st;
    if (stat(directory, &st) == -1) {
        if (errno == ENOENT) {
            /* Create directory with restrictive permissions */
            if (mkdir(directory, 0700) == -1) {
                return;
            }
        } else {
            return;
        }
    } else {
        if (!S_ISDIR(st.st_mode)) {
            /* The path exists but is not a directory */
            return;
        }
    }

    /* Build full path for the file */
    size_t dir_len = strlen(directory);
    size_t fname_len = strlen(filename);
    int need_slash = (dir_len == 0 || directory[dir_len - 1] != '/') ? 1 : 0;
    size_t path_len = dir_len + need_slash + fname_len + 1; /* +1 for NUL terminator */

    char *path = (char *)malloc(path_len);
    if (path == NULL) {
        return;
    }

    /* Construct path */
    memcpy(path, directory, dir_len);
    size_t pos = dir_len;
    if (need_slash) {
        path[pos++] = '/';
    }
    memcpy(path + pos, filename, fname_len);
    pos += fname_len;
    path[pos] = '\0';

    /* Open (create/truncate) file with restrictive permissions; avoid following symlinks if supported */
    int flags = O_WRONLY | O_CREAT | O_TRUNC;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif

    int fd = open(path, flags, 0600);
    if (fd == -1) {
        free(path);
        return;
    }

    /* Ensure permissions are 0600 even if the file existed previously */
    (void)fchmod(fd, 0600);

    /* Write content (empty string if content is NULL) */
    const char *buf = (content != NULL) ? content : "";
    size_t to_write = strlen(buf);
    ssize_t written_total = 0;

    while ((size_t)written_total < to_write) {
        ssize_t w = write(fd, buf + written_total, to_write - (size_t)written_total);
        if (w < 0) {
            /* Write failed; break and close */
            break;
        }
        written_total += w;
    }

    if ((size_t)written_total == to_write) {
        /* Attempt to flush to disk; ignore errors */
        (void)fsync(fd);
    }

    /* Close and free resources */
    (void)close(fd);
    free(path);
    return;
}
