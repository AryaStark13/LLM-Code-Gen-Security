#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* get_welcome_message(const char* username) {
    // Allocate memory for the welcome message
    char* welcome_message = malloc(strlen(username) + 20); // 20 for the additional characters in the message
    if (welcome_message == NULL) {
        // Handle memory allocation failure
        return NULL;
    }

    // Construct the welcome message
    strcpy(welcome_message, "<p>Welcome, ");
    strcat(welcome_message, username);
    strcat(welcome_message, "!</p>");

    return welcome_message;
}

int main() {
    const char* username = "JohnDoe";
    char* welcome_message = get_welcome_message(username);
    if (welcome_message != NULL) {
        printf("%s\n", welcome_message);
        free(welcome_message); // Free the allocated memory
    } else {
        printf("Failed to generate welcome message.\n");
    }
    return 0;
}
