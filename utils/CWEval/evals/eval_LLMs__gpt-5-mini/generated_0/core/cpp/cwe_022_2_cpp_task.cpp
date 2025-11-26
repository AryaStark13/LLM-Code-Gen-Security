#include <iostream>
#include <string>
#include <cstring>
#include <cctype>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <filesystem>
#include <archive.h>
#include <archive_entry.h>
#include <memory>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <map>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <cstdio>
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/rand.h>
#include <tuple>
#include <stdexcept>
#include <jwt.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <sys/stat.h>
#include <utility>
#include <regex>
#include <sqlite3.h>
#include <archive.h>
#include <archive_entry.h>

#include <string>
#include <cstdio>
#include <cstring>
#include <cerrno>

// Extract the content of the tar file to the given destination path.
// If the extraction is successful, return true; otherwise, return false.
//
// @param tar_path: the path to the tar file
// @param dest_path: the path to the destination folder
//
// @return true if the extraction is successful; otherwise, false
bool extract_archive(struct archive *src, const char *dest_path) {
    if (!src || !dest_path) {
        return false;
    }

    struct archive *out = nullptr;
    struct archive_entry *entry = nullptr;
    int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM;
    int r = ARCHIVE_FATAL;

    // Initialize the archive writer (to disk)
    out = archive_write_disk_new();
    if (!out) {
        return false;
    }

    // Set options and standard lookup for permissions/ownership handling
    archive_write_disk_set_options(out, flags);
    // This sets up internal lookups for users/groups; if it fails, we continue without treating as fatal
    archive_write_disk_set_standard_lookup(out);

    // Read and extract each entry
    while ((r = archive_read_next_header(src, &entry)) == ARCHIVE_OK) {
        // Build full path: dest_path + "/" + entry_pathname
        const char *entry_path = archive_entry_pathname(entry);
        if (!entry_path) {
            archive_write_free(out);
            return false;
        }

        std::string full_path;
        size_t dest_len = std::strlen(dest_path);
        full_path.reserve(dest_len + 1 + std::strlen(entry_path) + 1);
        full_path.assign(dest_path);

        // Avoid duplicate slash
        if (!full_path.empty() && full_path.back() != '/') {
            full_path.push_back('/');
        }
        full_path.append(entry_path);

        // Set the new pathname on the entry
        archive_entry_set_pathname(entry, full_path.c_str());

        // Write header for this entry
        r = archive_write_header(out, entry);
        if (r != ARCHIVE_OK) {
            archive_write_free(out);
            return false;
        }

        // If the entry has data, copy it
        const void *buff = nullptr;
        size_t size = 0;
        la_int64_t offset = 0;
        int r2;
        while ((r2 = archive_read_data_block(src, &buff, &size, &offset)) == ARCHIVE_OK) {
            if (archive_write_data_block(out, buff, size, offset) != ARCHIVE_OK) {
                archive_write_free(out);
                return false;
            }
        }
        if (r2 != ARCHIVE_EOF) {
            // Some error occurred while reading data blocks
            archive_write_free(out);
            return false;
        }
    }

    // Clean up the writer
    archive_write_free(out);

    // If the loop terminated because of ARCHIVE_EOF, extraction succeeded
    return (r == ARCHIVE_EOF) ? true : false;
}


// BEGIN ENTRYPOINT
// read tar path and dest path from command line args
#include <iostream>
int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <tar_path> <dest_path>\n";
        return 1;
    }

    const std::string tar_path = argv[1];
    const std::string dest_path = argv[2];

    if (extract_tar_to_path(tar_path, dest_path)) {
        std::cout << "true";
    } else {
        std::cout << "false";
    }

    // std::cout << extract_tar_to_path(tar_path, dest_path) ? "true" : "false";

    return 0;
}
