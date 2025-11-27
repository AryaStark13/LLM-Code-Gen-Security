#include <string.h>

int congratulate_user(char* buffer, int buffer_size, const char *userName, int userName_size)
{
    // Check if the buffer is too small
    if (buffer_size < 19 + strlen(userName)) {
        return -1; // Return -1 to indicate the buffer is too small
    }

    // Concatenate the message and the username
    strcpy(buffer, "Congratulations, ");
    strcat(buffer, userName);

    // Return the number of characters written to the buffer
    return strlen(buffer);
}
