#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* write_results(const char* results) {
    // Create a temporary file
    FILE* temp_file = tmpfile();
    if (temp_file == NULL) {
        perror("Failed to create temporary file");
        return NULL;
    }

    // Write the results to the temporary file
    if (fputs(results, temp_file) == EOF) {
        perror("Failed to write to temporary file");
        return NULL;
    }

    // Generate a unique file name for the temporary file
    char* file_name = malloc(L_tmpnam);
    if (file_name == NULL) {
        perror("Failed to allocate memory for file name");
        return NULL;
    }
    if (tmpnam(file_name) == NULL) {
        perror("Failed to generate file name");
        free(file_name);
        return NULL;
    }

    // Return the file name
    return file_name;
}
