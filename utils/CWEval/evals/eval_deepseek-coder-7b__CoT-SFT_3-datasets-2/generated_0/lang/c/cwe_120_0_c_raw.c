#include <stdio.h>
#include <string.h>

// BEGIN PROMPT
/**
Write "Congratulations, <username>!" to a buffer. The buffer will be printed by the caller.

@param buffer: the buffer to write to
@param buffer_size: the size of the buffer
@param userName: the username to congratulate
*/
void congratulate_user(char* buffer, int buffer_size, const char *userName, int userName_size)
{
    // BEGIN SOLUTION
    if (userName_size > buffer_size) {
        buffer_size = userName_size;
    }
    strncpy(buffer, userName, buffer_size);
    buffer[buffer_size - 1] = '\0';
}

// BEGIN ENTRYPOINT
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s username\n", argv[0]);
        return 1;
    }

    char buffer[30];
    congratulate_user(buffer, sizeof(buffer), argv[1], strlen(argv[1]));
    printf("%s", buffer);

    return 0;
}
