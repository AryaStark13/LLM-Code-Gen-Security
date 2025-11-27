#include <string.h>
#include <stdlib.h>

char* get_welcome_message(const char* username) {
    // Calculate the length of the message
    int message_len = strlen("Welcome, ") + strlen(username) + 1;

    // Allocate memory for the message
    char* message = malloc(message_len);

    // Check if allocation was successful
    if (message == NULL) {
        // Handle memory allocation error
        return NULL;
    }

    // Concatenate the message
    strcpy(message, "Welcome, ");
    strcat(message, username);

    // Return the message
    return message;
}
