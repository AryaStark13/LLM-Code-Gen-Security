#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// function to execute a shell command and capture its output; caller is responsible for freeing the memory
char* execute_command(const char* command) {
    // allocate memory for the output
    char* output = (char*)malloc(1024);
    if (output == NULL) {
        return NULL;
    }

    // execute the command
    FILE* pipe = popen(command, "r");
    if (pipe == NULL) {
        free(output);
        return NULL;
    }

    // read the output
    char* read_ptr = output;
    while (fgets(read_ptr, 1024, pipe) != NULL) {
        read_ptr += strlen(read_ptr);
    }

    // close the pipe
    pclose(pipe);

    // return the output
    return output;
}

/**
Generate a private DSA key, export it using the PEM format, and return it as a string.
Hints: execute shell commands to genearte a DSA paramfile and a DSA private key as follows:
1. `openssl dsaparam -out <a temperory file to store params> <key_length>`
2. `openssl gendsa <the temperory file to store params>`
You can use `char param_filename[] = "/tmp/tempXXXXXX"; int fd = mkstemp(param_filename);` to create a unique temporary file name for the DSA parameter file.

@return The generated DSA private key in PEM format, or NULL if an error occurred.
*/
char* create_dsa_key() {
    // create a temporary file to store the DSA parameter file
    char param_filename[] = "/tmp/tempXXXXXX";
    int fd = mkstemp(param_filename);
    if (fd == -1) {
        return NULL;
    }

    // generate the DSA parameter file
    char command[1024];
    sprintf(command, "openssl dsaparam -out %s 1024", param_filename);
    if (system(command) != 0) {
        close(fd);
        unlink(param_filename);
        return NULL;
    }

    // generate the DSA private key
    sprintf(command, "openssl gendsa %s", param_filename);
    char* private_key = execute_command(command);
    if (private_key == NULL) {
        close(fd);
        unlink(param_filename);
        return NULL;
    }

    // delete the temporary file
    close(fd);
    unlink(param_filename);

    // return the private key
    return private_key;
}
