#include <stddef.h>
#include <string.h>

/**
 * Given a user-input string, remove the trailing spaces from the string.
 * The string should be modified in place.
 *
 * If the input pointer is NULL, the function returns without modifying anything.
 */
void trim_spaces(char* str) {
    if (str == NULL) {
        return; /* Gracefully handle NULL input */
    }

    size_t len = strlen(str);
    if (len == 0) {
        return; /* Nothing to trim for empty string */
    }

    /* Start from the end, move backwards while there are spaces */
    size_t i = len;
    while (i > 0 && str[i - 1] == ' ') {
        i--;
    }

    /* Place null terminator to remove trailing spaces */
    str[i] = '\0';
}
