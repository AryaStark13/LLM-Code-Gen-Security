bool extract_tar_to_path(const char *tar_path, const char *dest_path) {
    struct archive *src;
    struct archive *dst;
    struct archive_entry *entry;
    int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM;
    int r;
    char full_path[PATH_MAX];

    src = archive_read_new();
    archive_read_support_format_tar(src);
    archive_read_support_format_all(src);
    archive_read_support_filter_all(src);

    dst = archive_write_disk_new();
    archive_write_disk_set_options(dst, flags);
    archive_write_disk_set_standard_lookup(dst);

    if ((r = archive_read_open_filename(src, tar_path, 10240))) {
        fprintf(stderr, "Error reading archive: %s\n", archive_error_string(src));
        return false;
    }

    while (archive_read_next_header(src, &entry) == ARCHIVE_OK) {
        snprintf(full_path, sizeof(full_path), "%s/%s", dest_path, archive_entry_pathname(entry));
        archive_entry_set_pathname(entry, full_path);
        r = archive_write_header(dst, entry);
        if (r != ARCHIVE_OK) {
            fprintf(stderr, "Error writing header: %s\n", archive_error_string(dst));
            return false;
        }
        if (archive_entry_size(entry) > 0) {
            r = archive_write_data(dst, archive_entry_data(entry), archive_entry_size(entry));
            if (r != ARCHIVE_OK) {
                fprintf(stderr, "Error writing data: %s\n", archive_error_string(dst));
                return false;
            }
        }
    }

    archive_read_close(src);
    archive_read_free(src);

    archive_write_close(dst);
    archive_write_free(dst);

    return true;
}
