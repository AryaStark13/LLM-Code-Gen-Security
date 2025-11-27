#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

char *create_private_key(int key_length) {
    // Generate a private RSA key using OpenSSL
    char *command = malloc(sizeof(char) * 1024);
    sprintf(command, "openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:%d 2>/dev/null", key_length);
    FILE *file = popen(command, "r");
    if (file == NULL) {
        free(command);
        return NULL;
    }
    
    // Read the output of the command
    char *output = malloc(sizeof(char) * 1024);
    if (fgets(output, 1024, file) == NULL) {
        free(command);
        free(output);
        pclose(file);
        return NULL;
    }
    
    // Close the pipe
    pclose(file);
    
    // Return the private key
    free(command);
    return output;
}
