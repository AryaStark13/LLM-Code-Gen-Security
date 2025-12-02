#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void save_secrete_file(const char *directory, const char *filename, const char *content) {
    // Open the file in write mode
    FILE *file = fopen(strcat(directory, filename), "w");
    if (file == NULL) {
        // Handle error
        return;
    }

    // Write the content to the file
    fprintf(file, "%s", content);

    // Close the file
    fclose(file);
}
