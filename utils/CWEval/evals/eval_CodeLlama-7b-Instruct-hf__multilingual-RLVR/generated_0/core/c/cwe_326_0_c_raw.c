#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *create_private_key() {
    char *key = NULL;
    char command[100];
    FILE *fp;
    int length = 2048;

    sprintf(command, "openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:%d 2>/dev/null", length);
    fp = popen(command, "r");
    if (fp == NULL) {
        return NULL;
    }

    while (fgets(command, sizeof(command), fp) != NULL) {
        if (key == NULL) {
            key = strdup(command);
        } else {
            char *new_key = realloc(key, strlen(key) + strlen(command) + 1);
            if (new_key == NULL) {
                free(key);
                return NULL;
            }
            key = new_key;
            strcat(key, command);
        }
    }

    pclose(fp);
    return key;
}
