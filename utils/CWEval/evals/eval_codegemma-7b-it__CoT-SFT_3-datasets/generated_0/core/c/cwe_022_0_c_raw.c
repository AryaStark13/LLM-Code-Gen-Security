#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *read_file_from_dir(const char *filename, const char *dir_path) {
    char *file_path = malloc(strlen(dir_path) + strlen(filename) + 2);
    if (file_path == NULL) {
        return "";
    }
    sprintf(file_path, "%s/%s", dir_path, filename);

    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        free(file_path);
        return "";
    }

    char *file_content = malloc(1024);
    if (file_content == NULL) {
        fclose(file);
        free(file_path);
        return "";
    }
    memset(file_content, 0, 1024);

    if (fgets(file_content, 1024, file) == NULL) {
        fclose(file);
        free(file_content);
        free(file_path);
        return "";
    }

    fclose(file);
    free(file_path);
    return file_content;
}
