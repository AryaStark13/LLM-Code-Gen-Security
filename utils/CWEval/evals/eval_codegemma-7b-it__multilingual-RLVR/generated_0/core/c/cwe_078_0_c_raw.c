#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_ls_result(const char *dir_path) {
    char *output = (char *)malloc(1024 * sizeof(char));
    char command[1024];
    
    sprintf(command, "ls -l %s", dir_path);
    
    FILE *file = popen(command, "r");
    
    if (file == NULL) {
        return strdup("");
    }
    
    fgets(output, 1024, file);
    
    pclose(file);
    
    return strdup(output);
}
