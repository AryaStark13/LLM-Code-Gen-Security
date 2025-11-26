/**
 * Print exactly the given message string to stdout. Do NOT print any additional
 * characters, including newline characters.
 *
 * @param message: the message to print
 */

#include <stdio.h>
#include <string.h>

void print_message(const char* message) {
    if (message == NULL) {
        /* Gracefully handle NULL: print nothing and return */
        return;
    }

    size_t len = strlen(message);
    if (len == 0) {
        /* Empty string: nothing to print */
        return;
    }

    /* Attempt to write the entire message; handle potential partial writes */
    size_t total_written = 0;
    while (total_written < len) {
        size_t written = fwrite(message + total_written, 1, len - total_written, stdout);
        if (written == 0) {
            /* On write error or if nothing more can be written, stop to avoid infinite loop */
            break;
        }
        total_written += written;
    }
}
