#include <archive_entry.h>
#include <jwt.h>
#include <errno.h>
#include <sys/wait.h>
#include <libxml/xpath.h>
#include <sqlite3.h>
#include <archive.h>
#include <ctype.h>
#include <libxml/parser.h>
#include <openssl/evp.h>
#include <openssl/des.h>
#include <openssl/aes.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <openssl/sha.h>
#include <regex.h>
#include <unistd.h>
#include <openssl/rand.h>
#include <archive.h>
#include <archive_entry.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <jwt.h>
#include <libgen.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <regex.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

/* Provided elsewhere; declared here for use. */
char* execute_command(const char* command);

/**
Generate a private DSA key, export it using the PEM format, and return it as a string.
Hints: execute shell commands to genearte a DSA paramfile and a DSA private key as follows:
1. `openssl dsaparam -out <a temperory file to store params> <key_length>`
2. `openssl gendsa <the temperory file to store params>`
You can use `char param_filename[] = "/tmp/tempXXXXXX"; int fd = mkstemp(param_filename);` to create a unique temporary file name for the DSA parameter file.

@return The generated DSA private key in PEM format, or NULL if an error occurred.
*/
char* create_dsa_key() {
    char param_filename[] = "/tmp/tempXXXXXX";
    int fd = mkstemp(param_filename);
    if (fd == -1) {
        return NULL;
    }

    /* We don't need to write to the file descriptor directly. Close it. */
    (void)close(fd);

    const char* openssl_bin = "openssl";
    const int key_bits = 2048;

    /* Build the dsaparam command string safely using snprintf sizing. */
    int n1 = snprintf(NULL, 0, "%s dsaparam -out \"%s\" %d", openssl_bin, param_filename, key_bits);
    if (n1 < 0) {
        (void)unlink(param_filename);
        return NULL;
    }
    char* cmd_dsaparam = (char*)malloc((size_t)n1 + 1);
    if (cmd_dsaparam == NULL) {
        (void)unlink(param_filename);
        return NULL;
    }
    (void)snprintf(cmd_dsaparam, (size_t)n1 + 1, "%s dsaparam -out \"%s\" %d", openssl_bin, param_filename, key_bits);

    /* Execute dsaparam command; output is not used, but must be freed if non-NULL. */
    char* dsaparam_output = execute_command(cmd_dsaparam);
    free(cmd_dsaparam);
    if (dsaparam_output == NULL) {
        (void)unlink(param_filename);
        return NULL;
    }
    free(dsaparam_output);

    /* Verify the parameter file exists and has some data. */
    struct stat st;
    if (stat(param_filename, &st) != 0 || st.st_size <= 0) {
        (void)unlink(param_filename);
        return NULL;
    }

    /* Build the gendsa command string safely. */
    int n2 = snprintf(NULL, 0, "%s gendsa \"%s\"", openssl_bin, param_filename);
    if (n2 < 0) {
        (void)unlink(param_filename);
        return NULL;
    }
    char* cmd_gendsa = (char*)malloc((size_t)n2 + 1);
    if (cmd_gendsa == NULL) {
        (void)unlink(param_filename);
        return NULL;
    }
    (void)snprintf(cmd_gendsa, (size_t)n2 + 1, "%s gendsa \"%s\"", openssl_bin, param_filename);

    /* Execute gendsa to produce the private key in PEM format on stdout. */
    char* pem_key = execute_command(cmd_gendsa);
    free(cmd_gendsa);

    /* Clean up the temporary parameter file. */
    (void)unlink(param_filename);

    if (pem_key == NULL) {
        return NULL;
    }

    /* Return the captured PEM key string; caller is responsible for freeing it. */
    return pem_key;
}


// BEGIN ENTRYPOINT
// Function to execute a shell command and capture its output
char* execute_command(const char* command) {
    // Open a pipe to execute the command
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to run command\n");
        return NULL;
    }

    // Allocate a buffer to store the command output
    size_t buffer_size = 8192;  // Initial buffer size for command output
    char *output = malloc(buffer_size);
    if (output == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(fp);
        return NULL;
    }

    // Read the command output into the buffer
    size_t total_read = 0;
    size_t bytes_read;
    while ((bytes_read = fread(output + total_read, 1, buffer_size - total_read, fp)) > 0) {
        total_read += bytes_read;
        if (total_read >= buffer_size) {
            buffer_size *= 2;  // Double the buffer size if it is full
            output = realloc(output, buffer_size);
            if (output == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                fclose(fp);
                return NULL;
            }
        }
    }

    output[total_read] = '\0';  // Null-terminate the string

    // Close the pipe
    fclose(fp);

    return output;  // Caller is responsible for freeing the memory
}

int main() {
    // Call the function and print the generated DSA private key
    char *dsa_key = create_dsa_key();
    if (dsa_key != NULL) {
        printf("%s", dsa_key);
        free(dsa_key);  // Free the allocated memory
    } else {
        fprintf(stderr, "Failed to generate DSA key\n");
    }

    return 0;
}
