#include <archive.h>
#include <archive_entry.h>
#include <string>

bool extract_tar_to_path(const std::string& tar_path, const std::string& dest_path) {
    struct archive *src = archive_read_new();
    if (!src) {
        return false;
    }
    
    if (archive_read_open_filename(src, tar_path.c_str()) != ARCHIVE_OK) {
        archive_read_free(src);
        return false;
    }
    
    struct archive *out = archive_write_disk_new();
    if (!out) {
        archive_read_free(src);
        return false;
    }
    
    int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM;
    archive_write_disk_set_options(out, flags);
    
    struct archive_entry *entry;
    int r;
    char full_path[4096];
    
    while ((r = archive_read_next_header(src, &entry)) == ARCHIVE_OK) {
        snprintf(full_path, sizeof(full_path), "%s/%s",
                 dest_path.c_str(), archive_entry_pathname(entry));
        archive_entry_set_pathname(entry, full_path);
        
        if (archive_write_header(out, entry) != ARCHIVE_OK) {
            archive_read_free(src);
            archive_write_free(out);
            return false;
        }
        
        const void *buff;
        size_t size;
        la_int64_t offset;
        
        while ((r = archive_read_data_block(src, &buff, &size, &offset)) == ARCHIVE_OK) {
            if (archive_write_data_block(out, buff, size, offset) != ARCHIVE_OK) {
                archive_read_free(src);
                archive_write_free(out);
                return false;
            }
        }
        
        if (r != ARCHIVE_EOF) {
            archive_read_free(src);
            archive_write_free(out);
            return false;
        }
    }
    
    archive_read_free(src);
    archive_write_free(out);
    
    return r == ARCHIVE_EOF ? true : false;
}
