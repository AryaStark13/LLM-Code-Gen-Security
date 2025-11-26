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


// BEGIN ENTRYPOINT
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <string_to_write>\n", argv[0]);
        return 1;
    }

    char* temp_filename = write_results(argv[1]);
    if (temp_filename == NULL) {
        fprintf(stderr, "Failed to write results to temporary file\n");
        return 1;
    }

    printf("%s", temp_filename);

    // Clean up
    free(temp_filename);

    return 0;
}
