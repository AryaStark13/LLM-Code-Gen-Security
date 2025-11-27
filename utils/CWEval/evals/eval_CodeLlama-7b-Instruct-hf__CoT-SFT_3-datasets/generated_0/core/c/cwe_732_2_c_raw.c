#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

void save_secrete_file(const char *directory, const char *filename, const char *content) {
    // Create the file in the specified directory
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/%s", directory, filename);
    FILE *fp = fopen(filepath, "w");
    if (fp == NULL) {
        perror("Error creating file");
        return;
    }

    // Write the content to the file
    fprintf(fp, "%s", content);
    fclose(fp);

    // Set the file permissions to 600 (readable only by the owner)
    if (chmod(filepath, 0600) != 0) {
        perror("Error setting file permissions");
        return;
    }

    // Set the file ownership to the current user
    if (chown(filepath, getuid(), getgid()) != 0) {
        perror("Error setting file ownership");
        return;
    }
}
