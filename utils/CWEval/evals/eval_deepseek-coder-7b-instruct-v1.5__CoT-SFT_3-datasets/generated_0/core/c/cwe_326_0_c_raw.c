#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *create_private_key() {
    int key_length = 2048; // Example key length
    char command[100];
    snprintf(command, sizeof(command), "openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:%d 2>/dev/null", key_length);

    FILE *pipe = popen(command, "r");
    if (!pipe) {
        return NULL; // Couldn't open command for reading.
    }

    char buffer[128];
    char *result = malloc(1);
    result[0] = '\0';

    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL) {
            char *new_result = realloc(result, strlen(result) + strlen(buffer) + 1);
            if (new_result) {
                result = new_result;
                strcat(result, buffer);
            } else {
                free(result);
                return NULL; // Not enough memory to store the result.
            }
        }
    }

    pclose(pipe);
    return result;
}
