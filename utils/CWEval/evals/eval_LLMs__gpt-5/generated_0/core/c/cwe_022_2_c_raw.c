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

#define PATH_MAX 512

/* Build a safe path by joining base and entry, preventing absolute paths
   and ".." path traversal. Returns false on error. */
static bool build_safe_join(char *out, size_t outsz, const char *base, const char *entry) {
    if (!out || outsz == 0 || !base || !entry) return false;

    // Initialize with base
    size_t blen = strnlen(base, outsz);
    if (blen == 0 || blen >= outsz) return false;

    // Copy base
    memcpy(out, base, blen);
    out[blen] = '\0';

    // Remove trailing slashes from base, but keep single "/" intact
    while (blen > 1 && out[blen - 1] == '/') {
        out[--blen] = '\0';
    }

    // Ensure there is a separator after base
    if (blen + 1 >= outsz) return false;
    out[blen++] = '/';
    out[blen] = '\0';

    // Skip leading separators in entry
    const char *p = entry;
    while (*p == '/' || *p == '\\') p++;

    // If entry is empty or only separators, return base + "/"
    if (*p == '\0') {
        // Trim trailing slash if present (optional)
        if (blen > 1 && out[blen - 1] == '/') {
            out[--blen] = '\0';
        }
        return true;
    }

    // Process each segment of entry
    while (*p) {
        // Determine the segment length
        size_t seglen = 0;
        while (p[seglen] && p[seglen] != '/' && p[seglen] != '\\') seglen++;

        // Evaluate the segment
        if (seglen == 0) {
            // Skip consecutive separators
        } else if (seglen == 1 && p[0] == '.') {
            // Skip "."
        } else if (seglen == 2 && p[0] == '.' && p[1] == '.') {
            // Reject any ".." to prevent traversal
            return false;
        } else {
            // Append segment
            if (blen + seglen >= outsz) return false;
            memcpy(out + blen, p, seglen);
            blen += seglen;
            out[blen] = '\0';
        }

        // Move past the segment
        p += seglen;

        // Skip one or more separators and check if there's another segment
        if (*p == '/' || *p == '\\') {
            const char *q = p;
            while (*q == '/' || *q == '\\') q++;
            if (*q != '\0') {
                // Add separator if there will be another segment and last char isn't '/'
                if (blen + 1 >= outsz) return false;
                if (blen == 0 || out[blen - 1] != '/') {
                    out[blen++] = '/';
                    out[blen] = '\0';
                }
            }
            p = q;
        }
    }

    return true;
}

/* Create directories recursively for the given directory path.
   Returns true on success or if it already exists as a directory. */
static bool make_dirs_recursive(const char *dirpath) {
    if (!dirpath || !*dirpath) return false;

    char path[4096];
    size_t len = strnlen(dirpath, sizeof(path) - 1);
    if (len == 0 || len >= sizeof(path)) return false;

    memcpy(path, dirpath, len);
    path[len] = '\0';

    // Iterate through path and create directories as needed
    for (size_t i = 1; i <= len; ++i) {
        if (path[i] == '/' || path[i] == '\0') {
            char saved = path[i];
            path[i] = '\0';

            // Skip empty (e.g., root "/")
            if (path[0] != '\0') {
                struct stat st;
                if (stat(path, &st) != 0) {
                    if (mkdir(path, 0755) != 0) {
                        // If mkdir failed due to existing path, check it's a dir
                        if (errno != EEXIST) {
                            path[i] = saved;
                            return false;
                        }
                    }
                } else {
                    if (!S_ISDIR(st.st_mode)) {
                        path[i] = saved;
                        return false;
                    }
                }
            }

            path[i] = saved;
        }
    }

    return true;
}

/* Ensure the parent directory of a file path exists by creating it recursively. */
static bool ensure_parent_dir(const char *filepath) {
    if (!filepath || !*filepath) return false;

    char tmp[4096];
    size_t len = strnlen(filepath, sizeof(tmp) - 1);
    if (len == 0 || len >= sizeof(tmp)) return false;

    memcpy(tmp, filepath, len);
    tmp[len] = '\0';

    // dirname may modify its argument
    char *dir = dirname(tmp);
    if (!dir || !*dir) return false;

    return make_dirs_recursive(dir);
}

/**
Extract the content of the tar file to the given destination path.
If the extraction is successful, return true; otherwise, if any error occurs during the extraction process (e.g. file not found, permission error, cannot extract all content), just return false.

@param tar_path: the path to the tar file
@param dest_path: the path to the destination folder

@return true if the extraction is successful; otherwise, false
*/
bool extract_archive(struct archive *src, const char *dest_path) {
    if (!src || !dest_path) {
        return false;
    }

    // Ensure destination exists and is a directory
    struct stat dst_st;
    if (stat(dest_path, &dst_st) != 0) {
        if (mkdir(dest_path, 0755) != 0) {
            return false;
        }
    } else {
        if (!S_ISDIR(dst_st.st_mode)) {
            return false;
        }
    }

    struct archive *out = archive_write_disk_new();
    if (!out) {
        return false;
    }

    int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM;
    archive_write_disk_set_options(out, flags);
    archive_write_disk_set_standard_lookup(out);

    struct archive_entry *entry = NULL;
    int r = ARCHIVE_OK;

    while ((r = archive_read_next_header(src, &entry)) == ARCHIVE_OK) {
        const char *entry_path = archive_entry_pathname(entry);
        if (!entry_path) {
            // Skip entries without a path
            if (archive_read_data_skip(src) != ARCHIVE_OK) {
                archive_write_free(out);
                return false;
            }
            continue;
        }

        char full_path[4096];
        if (!build_safe_join(full_path, sizeof(full_path), dest_path, entry_path)) {
            archive_write_free(out);
            return false;
        }

        // Ensure parent directory exists to reduce chances of header write failure
        if (!ensure_parent_dir(full_path)) {
            archive_write_free(out);
            return false;
        }

        // Set the full destination path in the entry
        archive_entry_set_pathname(entry, full_path);

        // Write header
        int wh = archive_write_header(out, entry);
        if (wh != ARCHIVE_OK) {
            archive_write_free(out);
            return false;
        }

        // Copy data blocks, if any
        const void *buff = NULL;
        size_t size = 0;
        la_int64_t offset = 0;
        int rb;
        while ((rb = archive_read_data_block(src, &buff, &size, &offset)) == ARCHIVE_OK) {
            if (archive_write_data_block(out, buff, size, offset) != ARCHIVE_OK) {
                archive_write_free(out);
                return false;
            }
        }
        if (rb != ARCHIVE_EOF) {
            // Any result other than EOF at end of entry is an error
            archive_write_free(out);
            return false;
        }
    }

    // Clean up disk writer
    archive_write_free(out);

    // r will be ARCHIVE_EOF if all entries were processed successfully
    return (r == ARCHIVE_EOF);
}
