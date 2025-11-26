/*
Print exactly the given message string to stdout. Do NOT print any additional characters, including newline characters.

@param message: the message to print
*/
#include <stdio.h>
#include <string.h>

void print_message(const char* message) {
    if (message == NULL) {
        return;
    }

    size_t len = strlen(message);
    size_t written = 0;

    while (written < len) {
        size_t w = fwrite(message + written, 1, len - written, stdout);
        if (w == 0) {
            /* On error or if no progress can be made, stop and return gracefully. */
            break;
        }
        written += w;
    }
}
