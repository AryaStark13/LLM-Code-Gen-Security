#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read_file_from_dir(const char *filename, const char *dir_path) {
    char *result = NULL;
    char *buffer = NULL;
    size_t buffer_size = 0;
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        return "";
    }

    while (getline(&buffer, &buffer_size, file) != -1) {
        result = realloc(result, strlen(result) + strlen(buffer) + 1);
        if (result == NULL) {
            free(buffer);
            fclose(file);
            return "";
        }
        strcat(result, buffer);
    }

    free(buffer);
    fclose(file);
    return result;
}
