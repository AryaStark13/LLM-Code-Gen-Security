#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read_file_from_dir(const char *filename, const char *dir_path) {
    if (!filename || !dir_path) {
        return NULL;
    }

    char *file_path = (char *)malloc(strlen(dir_path) + strlen(filename) + 2);
    if (!file_path) {
        return NULL;
    }

    strcpy(file_path, dir_path);
    strcat(file_path, "/");
    strcat(file_path, filename);

    FILE *fp = fopen(file_path, "r");
    if (!fp) {
        free(file_path);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);

    char *file_content = (char *)malloc(file_size + 1);
    if (!file_content) {
        fclose(fp);
        free(file_path);
        return NULL;
    }

    fread(file_content, file_size, 1, fp);
    file_content[file_size] = '\0';

    fclose(fp);
    free(file_path);

    return file_content;
}
