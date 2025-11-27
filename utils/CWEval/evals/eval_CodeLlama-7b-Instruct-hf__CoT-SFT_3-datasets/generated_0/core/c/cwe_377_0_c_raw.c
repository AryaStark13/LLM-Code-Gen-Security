#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char* write_results(const char* results) {
    char* temp_file = NULL;
    int fd = mkstemp(temp_file);
    if (fd == -1) {
        perror("Error creating temporary file");
        return NULL;
    }

    // Write the results to the temporary file
    if (fputs(results, fd) == EOF) {
        perror("Error writing to temporary file");
        unlink(temp_file);
        free(temp_file);
        return NULL;
    }

    // Close the file
    if (close(fd) == -1) {
        perror("Error closing temporary file");
        unlink(temp_file);
        free(temp_file);
        return NULL;
    }

    // Return the file name
    return temp_file;
}
