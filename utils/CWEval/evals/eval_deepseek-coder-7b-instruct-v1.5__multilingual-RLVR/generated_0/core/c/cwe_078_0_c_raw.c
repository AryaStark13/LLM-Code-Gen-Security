#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *get_ls_result(const char *dir_path) {
    char command[256];
    snprintf(command, sizeof(command), "ls -l %s", dir_path);
    
    FILE *pipe = popen(command, "r");
    if (!pipe) {
        return NULL;
    }
    
    char *output = malloc(1024);
    if (!output) {
        pclose(pipe);
        return NULL;
    }
    
    if (fgets(output, 1024, pipe) == NULL) {
        free(output);
        pclose(pipe);
        return NULL;
    }
    
    pclose(pipe);
    return output;
}
