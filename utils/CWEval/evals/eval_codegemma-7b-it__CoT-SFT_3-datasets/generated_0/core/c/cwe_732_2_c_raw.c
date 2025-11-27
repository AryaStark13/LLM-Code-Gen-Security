#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

int save_secrete_file(const char *directory, const char *filename, const char *content) {
    // Construct the full file path
    char *file_path = malloc(strlen(directory) + strlen(filename) + 2);
    if (file_path == NULL) {
        return -ENOMEM;
    }
    sprintf(file_path, "%s/%s", directory, filename);

    // Open the file
    FILE *file = fopen(file_path, "w");
    if (file == NULL) {
        free(file_path);
        return -errno;
    }

    // Write the content to the file
    if (fwrite(content, 1, strlen(content), file) != strlen(content)) {
        fclose(file);
        free(file_path);
        return -errno;
    }

    // Close the file
    if (fclose(file)) {
        free(file_path);
        return -errno;
    }

    // Set the file permissions to be accessible only by the current user
    if (chmod(file_path, 0600)) {
        free(file_path);
        return -errno;
    }

    // Free the file path
    free(file_path);

    // Return success
    return 0;
}
