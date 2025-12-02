#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read_file_from_dir(const char *filename, const char *dir_path) {
    // Allocate memory for the file content
    char *file_content = (char *)malloc(1024 * sizeof(char));
    
    // Construct the full file path
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, filename);
    
    // Read the file
    FILE *file = fopen(full_path, "r");
    if (file == NULL) {
        // File not found or other error
        free(file_content);
        return "";
    }
    
    // Read the file content
    fgets(file_content, 1024, file);
    
    // Close the file
    fclose(file);
    
    return file_content;
}
