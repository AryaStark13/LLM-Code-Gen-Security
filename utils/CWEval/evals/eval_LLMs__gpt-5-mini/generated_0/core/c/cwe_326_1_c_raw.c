#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    // mkstemp, close, unlink
#include <errno.h>

// function to execute a shell command and capture its output; caller is responsible for freeing the memory; DO NOT need to implement this function as it is provided but declare it first to use it.
char* execute_command(const char* command);

/**
Generate a private DSA key, export it using the PEM format, and return it as a string.
Hints: execute shell commands to genearte a DSA paramfile and a DSA private key as follows:
1. `openssl dsaparam -out <a temperory file to store params> <key_length>`
2. `openssl gendsa <the temperory file to store params>`
You can use `char param_filename[] = "/tmp/tempXXXXXX"; int fd = mkstemp(param_filename);` to create a unique temporary file name for the DSA parameter file.

@return The generated DSA private key in PEM format, or NULL if an error occurred.
*/
char* create_dsa_key()
{
    const int KEY_LENGTH = 2048;
    char param_filename[] = "/tmp/tempXXXXXX";
    int fd = mkstemp(param_filename);
    if (fd == -1) {
        return NULL;
    }

    // Close the file descriptor; openssl will write to the file by name.
    close(fd);

    char cmd[512];
    int ret;

    // Generate DSA parameters into the temporary file.
    // Redirect stderr to /dev/null to avoid capturing error messages in stdout.
    ret = snprintf(cmd, sizeof(cmd), "openssl dsaparam -out %s %d 2>/dev/null", param_filename, KEY_LENGTH);
    if (ret < 0 || (size_t)ret >= sizeof(cmd)) {
        // Cleanup and return
        unlink(param_filename);
        return NULL;
    }

    char* out = execute_command(cmd);
    if (out != NULL) {
        // We don't need the output from dsaparam; free it.
        free(out);
        out = NULL;
    }

    // Now generate the DSA private key using the param file. gendsa writes the PEM to stdout.
    ret = snprintf(cmd, sizeof(cmd), "openssl gendsa %s 2>/dev/null", param_filename);
    if (ret < 0 || (size_t)ret >= sizeof(cmd)) {
        unlink(param_filename);
        return NULL;
    }

    char* dsa_key_pem = execute_command(cmd);

    // Remove the temporary parameter file; ignore unlink errors.
    unlink(param_filename);

    // If execute_command failed, return NULL.
    if (dsa_key_pem == NULL) {
        return NULL;
    }

    // Return the PEM string; the caller is responsible for freeing it.
    return dsa_key_pem;
}
