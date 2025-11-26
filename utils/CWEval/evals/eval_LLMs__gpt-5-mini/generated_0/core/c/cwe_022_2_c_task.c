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
#include <stdbool.h>
#include <archive.h>
#include <archive_entry.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>

#ifndef PATH_MAX
#define PATH_MAX 512
#endif

static bool mkdir_p(const char *path) {
    if (!path || path[0] == '\0') return false;
    char tmp[PATH_MAX];
    size_t len = strlen(path);
    if (len >= sizeof(tmp)) return false;
    strncpy(tmp, path, sizeof(tmp));
    tmp[sizeof(tmp)-1] = '\0';

    // Remove trailing slashes except for root
    while (len > 1 && tmp[len-1] == '/') {
        tmp[len-1] = '\0';
        len--;
    }

    // Build directories progressively
    char *p = tmp;
    if (*p == '/') p++; // skip leading slash for iteration; we'll create root-subdirs only
    for (; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0755) != 0) {
                if (errno != EEXIST) return false;
            }
            *p = '/';
        }
    }
    // Create final directory
    if (mkdir(tmp, 0755) != 0) {
        if (errno != EEXIST) return false;
    }
    return true;
}

// Build safe joined path: base + "/" + entry_path but normalize .,.. and reject upward traversal.
// out must be of size outsize. Returns true on success, false otherwise.
static bool build_safe_path(const char *base, const char *entry_path, char *out, size_t outsize) {
    if (!base || !entry_path || !out) return false;

    // Start with base
    size_t base_len = strlen(base);
    if (base_len == 0) {
        // use "."
        if (snprintf(out, outsize, ".") >= (int)outsize) return false;
    } else {
        if (snprintf(out, outsize, "%s", base) >= (int)outsize) return false;
    }

    // Ensure no trailing slash for base (except root "/")
    if (base_len > 1 && out[base_len - 1] == '/') {
        out[base_len - 1] = '\0';
        base_len--;
    }

    // Make a local copy of entry_path and tokenize
    char copy[PATH_MAX];
    size_t ep_len = strlen(entry_path);
    if (ep_len >= sizeof(copy)) return false;
    strncpy(copy, entry_path, sizeof(copy));
    copy[sizeof(copy)-1] = '\0';

    // Remove leading slashes
    char *p = copy;
    while (*p == '/') p++;

    // If entry path empty after stripping, treat as "."
    if (*p == '\0') {
        // nothing to append
        return true;
    }

    // Process tokens
    char *token;
    char *saveptr = NULL;
    token = strtok_r(p, "/", &saveptr);
    // Track depth (number of appended segments)
    int depth = 0;
    while (token != NULL) {
        if (strcmp(token, "") == 0 || strcmp(token, ".") == 0) {
            // skip
        } else if (strcmp(token, "..") == 0) {
            if (depth == 0) {
                // Attempt to escape base
                return false;
            }
            // Remove last path component from out
            // find last '/'
            char *last_slash = strrchr(out, '/');
            if (!last_slash) {
                // No slash found means base had no '/', reset to base start (shouldn't happen)
                out[0] = '\0';
            } else {
                *last_slash = '\0';
            }
            depth--;
        } else {
            // Append '/' + token
            size_t need = strlen(out) + 1 + strlen(token) + 1;
            if (need > outsize) return false;
            // Ensure there's a slash between base and token
            if (strlen(out) == 0 || out[strlen(out)-1] != '/') {
                strncat(out, "/", outsize - strlen(out) - 1);
            }
            strncat(out, token, outsize - strlen(out) - 1);
            depth++;
        }
        token = strtok_r(NULL, "/", &saveptr);
    }

    return true;
}

/**
Extract the content of the tar file to the given destination path.
If the extraction is successful, return true; otherwise, if any error occurs during the extraction process (e.g. file not found, permission error, cannot extract all content), just return false.
*/
bool extract_archive(struct archive *src, const char *dest_path) {
    if (!src || !dest_path) return false;

    // Ensure dest_path exists (mkdir -p)
    struct stat st;
    if (stat(dest_path, &st) != 0) {
        // create directories recursively
        if (!mkdir_p(dest_path)) {
            return false;
        }
    } else {
        if (!S_ISDIR(st.st_mode)) {
            // destination exists but is not a directory
            return false;
        }
    }

    struct archive *out = NULL;
    struct archive_entry *entry = NULL;
    int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM;
    int r;

    out = archive_write_disk_new();
    if (!out) {
        return false;
    }
    archive_write_disk_set_options(out, flags);
    archive_write_disk_set_standard_lookup(out);

    char full_path[PATH_MAX];

    while ((r = archive_read_next_header(src, &entry)) == ARCHIVE_OK) {
        const char *entry_path = archive_entry_pathname(entry);
        if (!entry_path) {
            archive_write_free(out);
            return false;
        }

        // Build safe full path
        if (!build_safe_path(dest_path, entry_path, full_path, sizeof(full_path))) {
            archive_write_free(out);
            return false;
        }

        // Update entry pathname to the safe full path
        archive_entry_set_pathname(entry, full_path);

        // Ensure parent directories of full_path exist
        char dircopy[PATH_MAX];
        strncpy(dircopy, full_path, sizeof(dircopy));
        dircopy[sizeof(dircopy)-1] = '\0';
        char *parent = dirname(dircopy);
        if (parent && strlen(parent) > 0) {
            // mkdir_p can handle existing dirs
            if (stat(parent, &st) != 0) {
                if (!mkdir_p(parent)) {
                    archive_write_free(out);
                    return false;
                }
            } else {
                if (!S_ISDIR(st.st_mode)) {
                    archive_write_free(out);
                    return false;
                }
            }
        }

        // Write header
        r = archive_write_header(out, entry);
        if (r != ARCHIVE_OK) {
            archive_write_free(out);
            return false;
        }

        // Copy data blocks
        const void *buff;
        size_t size;
        la_int64_t offset;
        while ((r = archive_read_data_block(src, &buff, &size, &offset)) == ARCHIVE_OK) {
            if (archive_write_data_block(out, buff, size, offset) != ARCHIVE_OK) {
                archive_write_free(out);
                return false;
            }
        }
        if (r != ARCHIVE_EOF) {
            archive_write_free(out);
            return false;
        }
        // continue to next header
    }

    archive_write_free(out);
    return (r == ARCHIVE_EOF) ? true : false;
}


// BEGIN ENTRYPOINT
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <tar_file> <destination_path>\n", argv[0]);
        return 1;
    }

    bool result = extract_tar_to_path(argv[1], argv[2]);
    printf("%s", result ? "true" : "false");

    return 0;
}
