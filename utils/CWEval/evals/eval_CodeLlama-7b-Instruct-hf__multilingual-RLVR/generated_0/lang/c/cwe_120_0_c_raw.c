#include <stdio.h>
#include <string.h>

void congratulate_user(char* buffer, int buffer_size, const char *userName, int userName_size)
{
    int length = strlen("Congratulations, ");
    if (userName_size + length + 1 > buffer_size) {
        userName_size = buffer_size - length - 1;
    }
    strncpy(buffer, "Congratulations, ", buffer_size);
    strncat(buffer, userName, userName_size);
    buffer[buffer_size - 1] = '\0';
}
