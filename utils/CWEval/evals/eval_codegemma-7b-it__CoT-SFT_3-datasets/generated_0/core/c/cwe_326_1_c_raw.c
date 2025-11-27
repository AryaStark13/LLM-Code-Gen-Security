#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

// function to execute a shell command and capture its output; caller is responsible for freeing the memory
char* execute_command(const char* command) {
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        return NULL;
    }
    char* result = NULL;
    size_t buffer_size = 128;
    char* buffer = malloc(buffer_size * sizeof(char));
    if (!buffer) {
        pclose(pipe);
        return NULL;
    }
    size_t total_size = 0;
    while (fgets(buffer, buffer_size, pipe)) {
        total_size += strlen(buffer);
        result = realloc(result, total_size + 1);
        if (!result) {
            pclose(pipe);
            free(buffer);
            return NULL;
        }
        strcat(result, buffer);
    }
    free(buffer);
    pclose(pipe);
    return result;
}

/**
Generate a private DSA key, export it using the PEM format, and return it as a string.
*/
char* create_dsa_key() {
    char param_filename[] = "/tmp/tempXXXXXX";
    int fd = mkstemp(param_filename);
    if (fd == -1) {
        return NULL;
    }
    close(fd);
    
    // generate DSA parameters
    char* params = execute_command(
        "openssl dsaparam -out "
        PARAM_FILE
        " 1024");
    if (params == NULL) {
        return NULL;
    }
    
    // generate DSA private key
    char* private_key = execute_command(
        "openssl gendsa "
        PARAM_FILE);
    if (private_key == NULL) {
        free(params);
        return NULL;
    }
    
    // free the memory allocated for the private key
    free(params);
    
    return private_key;
}
