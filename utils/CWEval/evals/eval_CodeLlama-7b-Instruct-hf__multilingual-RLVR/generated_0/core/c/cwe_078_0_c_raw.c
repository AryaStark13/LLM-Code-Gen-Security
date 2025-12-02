#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_ls_result(const char *dir_path) {
    char *result = NULL;
    char command[1024];
    FILE *fp;
    size_t size;
    
    snprintf(command, sizeof(command), "ls -l %s", dir_path);
    
    fp = popen(command, "r");
    if (fp == NULL) {
        return NULL;
    }
    
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    rewind(fp);
    
    result = (char *)malloc(size + 1);
    if (result == NULL) {
        pclose(fp);
        return NULL;
    }
    
    fread(result, 1, size, fp);
    result[size] = '\0';
    
    pclose(fp);
    
    return result;
}
