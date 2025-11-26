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
#include <filesystem>
#include <system_error>

// Extract the content of the archive represented by 'src' into 'dest_path'.
// Returns true on success; false on any error.
bool extract_archive(struct archive *src, const char *dest_path) {
    // Validate inputs
    if (src == nullptr || dest_path == nullptr || *dest_path == '\0') {
        return false;
    }

    // Ensure destination directory exists
    std::filesystem::path root(dest_path);
    {
        std::error_code ec;
        if (!std::filesystem::exists(root, ec)) {
            if (ec) {
                return false;
            }
            if (!std::filesystem::create_directories(root, ec)) {
                return false;
            }
        } else {
            if (!std::filesystem::is_directory(root, ec)) {
                return false;
            }
        }
    }

    // Initialize the archive writer
    struct archive *out = archive_write_disk_new();
    if (!out) {
        return false;
    }

    // Set extraction options (keep it minimal to avoid requiring elevated privileges)
    int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM;
    archive_write_disk_set_options(out, flags);
    archive_write_disk_set_standard_lookup(out);

    struct archive_entry *entry = nullptr;
    int r = ARCHIVE_OK;

    // Iterate through all entries in the archive
    while (true) {
        r = archive_read_next_header(src, &entry);
        if (r == ARCHIVE_EOF) {
            break; // Finished reading entries
        }
        if (r != ARCHIVE_OK) {
            archive_write_free(out);
            return false;
        }

        // Get entry path
        const char *entryPath = archive_entry_pathname(entry);
        if (!entryPath) {
            entryPath = "";
        }

        // Build the output path safely
        std::filesystem::path inPath(entryPath);

        // Normalize and avoid absolute paths
        std::filesystem::path rel = inPath.is_absolute() ? inPath.relative_path() : inPath;
        rel = rel.lexically_normal();

        // Basic protection against path traversal (skip entries containing "..")
        bool containsParent = false;
        for (const auto &part : rel) {
            if (part == "..") {
                containsParent = true;
                break;
            }
        }
        if (containsParent) {
            // Skip potentially unsafe entries
            continue;
        }

        std::filesystem::path full = root / rel;
        std::string fullStr = full.string();

        // Update the entry's target pathname
        archive_entry_set_pathname(entry, fullStr.c_str());

        // Ensure parent directories exist before writing the header
        if (rel.has_parent_path()) {
            std::filesystem::path parent = full.parent_path();
            if (!parent.empty()) {
                std::error_code ec;
                if (!std::filesystem::exists(parent, ec)) {
                    if (!std::filesystem::create_directories(parent, ec)) {
                        archive_write_free(out);
                        return false;
                    }
                } else if (!std::filesystem::is_directory(parent, ec)) {
                    archive_write_free(out);
                    return false;
                }
            }
        }

        // Write the entry header
        r = archive_write_header(out, entry);
        if (r != ARCHIVE_OK) {
            archive_write_free(out);
            return false;
        }

        // Copy data blocks, if any
        const void *buff = nullptr;
        size_t size = 0;
        la_int64_t offset = 0;

        while (true) {
            r = archive_read_data_block(src, &buff, &size, &offset);
            if (r == ARCHIVE_EOF) {
                break; // End of this entry's data
            }
            if (r != ARCHIVE_OK) {
                archive_write_free(out);
                return false;
            }
            r = archive_write_data_block(out, buff, size, offset);
            if (r != ARCHIVE_OK) {
                archive_write_free(out);
                return false;
            }
        }

        // Finish writing this entry
        r = archive_write_finish_entry(out);
        if (r != ARCHIVE_OK) {
            archive_write_free(out);
            return false;
        }
    }

    // Clean up writer
    archive_write_free(out);

    // If we exited the loop with EOF, extraction is successful
    return true;
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
