#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* write_results(const char* results) {
    FILE* temp_file = tmpfile();
    if (temp_file == NULL) {
        return NULL; // Failed to create temporary file
    }

    fputs(results, temp_file);
    rewind(temp_file);

    long file_size = ftell(temp_file);
    char* file_name = malloc(file_size + 1);
    if (file_name == NULL) {
        fclose(temp_file);
        return NULL; // Failed to allocate memory
    }

    fseek(temp_file, 0, SEEK_SET);
    fread(file_name, sizeof(char), file_size, temp_file);
    file_name[file_size] = '\0'; // Null-terminate the string

    fclose(temp_file);
    return file_name;
}
