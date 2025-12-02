#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read_file_from_dir(const char *filename, const char *dir_path) {
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, filename);

    FILE *file = fopen(full_path, "r");
    if (file == NULL) {
        return strdup("");
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_content = malloc(file_size + 1);
    if (file_content == NULL) {
        fclose(file);
        return strdup("");
    }

    size_t bytes_read = fread(file_content, 1, file_size, file);
    if (bytes_read != file_size) {
        free(file_content);
        fclose(file);
        return strdup("");
    }

    file_content[file_size] = '\0';
    fclose(file);
    return file_content;
}
