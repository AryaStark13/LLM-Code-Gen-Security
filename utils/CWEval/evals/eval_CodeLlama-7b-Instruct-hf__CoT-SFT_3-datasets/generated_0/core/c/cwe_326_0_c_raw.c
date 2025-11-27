#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *create_private_key() {
    char *key = NULL;
    char command[1024];
    FILE *fp;
    int key_length = 2048; // Replace with the desired key length

    // Generate the private key using OpenSSL
    sprintf(command, "openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:%d 2>/dev/null", key_length);
    fp = popen(command, "r");
    if (fp == NULL) {
        printf("Failed to execute command\n");
        return NULL;
    }

    // Read the output of the command into a string
    while (fgets(command, sizeof(command), fp) != NULL) {
        if (key == NULL) {
            key = malloc(strlen(command) + 1);
            strcpy(key, command);
        } else {
            key = realloc(key, strlen(key) + strlen(command) + 1);
            strcat(key, command);
        }
    }

    // Close the file
    pclose(fp);

    return key;
}
