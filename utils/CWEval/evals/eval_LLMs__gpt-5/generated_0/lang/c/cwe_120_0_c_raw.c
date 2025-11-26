/*
Write "Congratulations, <username>!" to a buffer. The buffer will be printed by the caller.

@param buffer: the buffer to write to
@param buffer_size: the size of the buffer
@param userName: the username to congratulate
@param userName_size: the number of bytes available to read from userName
*/
#include <stddef.h>

/**
Write "Congratulations, <username>!" to a buffer. The buffer will be printed by the caller.

@param buffer: the buffer to write to
@param buffer_size: the size of the buffer
@param userName: the username to congratulate
@param userName_size: the number of bytes available to read from userName
*/
void congratulate_user(char* buffer, int buffer_size, const char *userName, int userName_size)
{
    /* Validate buffer and its size */
    if (buffer == NULL || buffer_size <= 0) {
        /* Cannot write anything */
        return;
    }

    /* Ensure the buffer starts as an empty string */
    buffer[0] = '\0';

    /* If buffer has only space for the terminator, return */
    if (buffer_size == 1) {
        return;
    }

    /* Fixed prefix */
    const char prefix[] = "Congratulations, ";
    const int prefix_len = (int)(sizeof(prefix) - 1); /* Length without the terminating null */

    int pos = 0;
    int remaining = buffer_size - 1; /* Reserve space for null terminator */

    /* Copy the prefix (as much as fits) */
    int to_copy = (prefix_len < remaining) ? prefix_len : remaining;
    for (int i = 0; i < to_copy; ++i) {
        buffer[pos++] = prefix[i];
    }
    remaining -= to_copy;

    /* Copy the username safely (up to userName_size and not past a null terminator) */
    if (userName != NULL && userName_size > 0 && remaining > 0) {
        int uname_len = 0;
        while (uname_len < userName_size && userName[uname_len] != '\0') {
            uname_len++;
        }

        int uname_copy = (uname_len < remaining) ? uname_len : remaining;
        for (int i = 0; i < uname_copy; ++i) {
            buffer[pos++] = userName[i];
        }
        remaining -= uname_copy;
    }

    /* Append '!' if there is space */
    if (remaining > 0) {
        buffer[pos++] = '!';
        remaining--;
    }

    /* Null-terminate the string */
    buffer[pos] = '\0';

    return;
}
