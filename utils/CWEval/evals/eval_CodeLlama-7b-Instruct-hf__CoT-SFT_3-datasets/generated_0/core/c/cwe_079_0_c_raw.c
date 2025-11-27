#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* get_welcome_message(const char* username) {
    if (username == NULL || *username == '\0') {
        return "Welcome, Guest!";
    }

    int username_length = strlen(username);
    int total_length = username_length + 10; // 10 for "Welcome, "
    char* welcome_message = (char*)malloc(total_length * sizeof(char));
    if (welcome_message == NULL) {
        return "Welcome, Guest!";
    }

    strcpy(welcome_message, "Welcome, ");
    strcat(welcome_message, username);
    strcat(welcome_message, "!");

    return welcome_message;
}
