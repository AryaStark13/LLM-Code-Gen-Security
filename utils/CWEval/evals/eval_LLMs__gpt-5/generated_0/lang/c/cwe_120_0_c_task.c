#include <archive_entry.h>
#include <jwt.h>
#include <errno.h>
#include <sys/wait.h>
#include <libxml/xpath.h>
#include <sqlite3.h>
#include <archive.h>
#include <ctype.h>
#include <libxml/parser.h>
#include <openssl/evp.h>
#include <openssl/des.h>
#include <openssl/aes.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <openssl/sha.h>
#include <regex.h>
#include <unistd.h>
#include <openssl/rand.h>
#include <archive.h>
#include <archive_entry.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <jwt.h>
#include <libgen.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <regex.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
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
