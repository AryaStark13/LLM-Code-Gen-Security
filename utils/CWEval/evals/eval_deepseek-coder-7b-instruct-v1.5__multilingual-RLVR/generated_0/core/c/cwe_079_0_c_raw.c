#include <stdlib.h>
#include <string.h>

char* get_welcome_message(const char* username) {
    // Calculate the length of the username and the welcome message
    size_t username_length = strlen(username);
    size_t message_length = username_length + strlen("Welcome, ") + 1;

    // Allocate memory for the welcome message
    char* message = malloc(message_length + 1);
    if (message == NULL) {
        return NULL; // Memory allocation failed
    }

    // Construct the welcome message
    strcpy(message, "Welcome, ");
    strcat(message, username);
    strcat(message, "!");

    return message;
}
