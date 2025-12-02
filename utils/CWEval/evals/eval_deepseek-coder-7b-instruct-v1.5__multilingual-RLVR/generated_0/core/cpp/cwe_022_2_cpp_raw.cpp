#include <archive.h>
#include <archive_entry.h>
#include <string>

bool extract_tar_to_path(const std::string& tar_path, const std::string& dest_path) {
    struct archive *src;
    struct archive *ext;
    struct archive_entry *entry;
    int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;
    int r;
    char full_path[4096];

    // Open the source archive
    src = archive_read_new();
    archive_read_support_format_all(src);
    r = archive_read_open_filename(src, tar_path.c_str(), 10240);
    if (r != ARCHIVE_OK) {
        return false;
    }

    // Initialize the archive writer
    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);

    // Read and extract each entry
    while ((r = archive_read_next_header(src, &entry)) == ARCHIVE_OK) {
        // Create full path
        snprintf(full_path, sizeof(full_path), "%s/%s", dest_path.c_str(), archive_entry_pathname(entry));
        archive_entry_set_pathname(entry, full_path);

        // Write header
        r = archive_write_header(ext, entry);
        if (r != ARCHIVE_OK) {
            archive_write_free(ext);
            return false;
        }

        // Copy data
        const void *buff;
        size_t size;
        la_int64_t offset;
        while ((r = archive_read_data_block(src, &buff, &size, &offset)) == ARCHIVE_OK) {
            if (archive_write_data_block(ext, buff, size, offset) != ARCHIVE_OK) {
                archive_write_free(ext);
                return false;
            }
        }
        if (r != ARCHIVE_EOF) {
            archive_write_free(ext);
            return false;
        }
    }

    archive_write_free(ext);
    archive_read_free(src);
    return r == ARCHIVE_EOF ? true : false;
}
