#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

char *get_ls_result(const char *dir_path) {
    static char result[4096]; // Use a static buffer to store the result
    struct stat st;

    // Check if the directory exists and is accessible
    if (stat(dir_path, &st) == -1) {
        return ""; // Directory not found or inaccessible
    }

    // Check if the directory is a directory
    if (!S_ISDIR(st.st_mode)) {
        return ""; // Not a directory
    }

    // Construct the command to execute
    char command[256];
    snprintf(command, sizeof(command), "ls -l %s", dir_path);

    // Execute the command and capture the output
    int ret = system(command);
    if (ret == -1) {
        return ""; // Error executing the command
    }

    // Return the output of the command
    return result;
}
