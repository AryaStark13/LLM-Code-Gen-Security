#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read_file_from_dir(const char *filename, const char *dir_path) {
    char *file_path = malloc(strlen(dir_path) + strlen(filename) + 2);
    sprintf(file_path, "%s/%s", dir_path, filename);

    FILE *fp = fopen(file_path, "r");
    if (!fp) {
        free(file_path);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);

    char *file_content = malloc(file_size + 1);
    fread(file_content, file_size, 1, fp);
    file_content[file_size] = '\0';

    fclose(fp);
    free(file_path);

    return file_content;
}
