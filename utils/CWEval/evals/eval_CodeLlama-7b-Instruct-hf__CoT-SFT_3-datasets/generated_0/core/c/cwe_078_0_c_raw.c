#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_ls_result(const char *dir_path) {
    char *result = NULL;
    char buffer[1024];
    FILE *fp;
    size_t len = 0;
    ssize_t read;

    // Execute the command
    char command[1024];
    sprintf(command, "ls -l %s", dir_path);
    if (system(command) != 0) {
        // Command execution failed
        return result;
    }

    // Read the output
    fp = popen(command, "r");
    if (fp == NULL) {
        // Failed to open pipe for reading
        return result;
    }

    // Read the output line by line
    while ((read = fgets(buffer, sizeof(buffer), fp)) != NULL) {
        len += strlen(buffer);
        result = realloc(result, len + 1);
        strcat(result, buffer);
    }

    // Close the pipe
    pclose(fp);

    return result;
}
