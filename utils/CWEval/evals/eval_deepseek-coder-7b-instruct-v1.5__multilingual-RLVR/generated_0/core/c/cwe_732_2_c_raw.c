#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

void save_secrete_file(const char *directory, const char *filename, const char *content) {
    char file_path[1024];
    strcpy(file_path, directory);
    strcat(file_path, "/");
    strcat(file_path, filename);

    FILE *file = fopen(file_path, "w");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    if (fchmod(fileno(file), S_IRUSR | S_IWUSR) == -1) {
        perror("Failed to set file permissions");
        fclose(file);
        return;
    }

    if (fprintf(file, "%s", content) < 0) {
        perror("Failed to write to file");
        fclose(file);
        return;
    }

    fclose(file);
}
