#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/**
Create a temporary file, write the provided results to it, and return the file name for further reading.

@param results The string content to write to the temporary file
@return A dynamically allocated string containing the temporary file name, or NULL on failure.
*/
char* write_results(const char* results) {
    const char* src = results ? results : "";
    size_t len = strlen(src);

    /* Template for mkstemp - mkstemp modifies this buffer in place */
    char tmpl[] = "/tmp/results_XXXXXX";
    int fd = mkstemp(tmpl);
    if (fd == -1) {
        /* Failed to create temporary file */
        return NULL;
    }

    /* Write the full buffer, handling partial writes and EINTR */
    size_t written = 0;
    const char* buf = src;
    while (written < len) {
        ssize_t w = write(fd, buf + written, len - written);
        if (w < 0) {
            if (errno == EINTR) {
                continue;
            }
            /* Write failure: clean up and return NULL */
            close(fd);
            unlink(tmpl);
            return NULL;
        }
        written += (size_t)w;
    }

    /* Optionally flush to disk; ignore fsync failure but continue */
    (void)fsync(fd);

    if (close(fd) == -1) {
        /* If close fails, we still try to return the filename; nothing more to do */
    }

    /* Allocate and return a copy of the filename */
    size_t path_len = strlen(tmpl) + 1;
    char* result_path = (char*)malloc(path_len);
    if (!result_path) {
        /* Allocation failure: remove the file and return NULL */
        unlink(tmpl);
        return NULL;
    }
    memcpy(result_path, tmpl, path_len);
    return result_path;
}
