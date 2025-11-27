#include <string.h>

bool congratulate_user(char* buffer, int buffer_size, const char *userName, int userName_size)
{
    // Check if the buffer size is sufficient to hold the message
    if (buffer_size < 15 + userName_size + 1) {
        return false; // Not enough space to write the message
    }

    // Write the message to the buffer
    strcpy(buffer, "Congratulations, ");
    strcpy(buffer + 15, userName);

    return true; // Message successfully written to the buffer
}
