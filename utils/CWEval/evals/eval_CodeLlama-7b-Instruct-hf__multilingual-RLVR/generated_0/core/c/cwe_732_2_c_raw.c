#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

void save_secrete_file(const char *directory, const char *filename, const char *content) {
    int fd;
    char path[256];

    snprintf(path, sizeof(path), "%s/%s", directory, filename);

    fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Failed to open file");
        return;
    }

    if (write(fd, content, strlen(content)) == -1) {
        perror("Failed to write to file");
    }

    if (close(fd) == -1) {
        perror("Failed to close file");
    }
}
