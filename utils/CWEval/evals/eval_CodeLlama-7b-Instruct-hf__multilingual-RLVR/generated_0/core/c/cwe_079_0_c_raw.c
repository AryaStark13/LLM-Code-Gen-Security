#include <stdlib.h>
#include <string.h>

char* get_welcome_message(const char* username) {
    if (!username) {
        return NULL;
    }

    size_t len = strlen(username);
    char* message = malloc(len + 10); // 10 for "Welcome, "
    if (!message) {
        return NULL;
    }

    strcpy(message, "Welcome, ");
    strcat(message, username);
    strcat(message, "!");

    return message;
}
