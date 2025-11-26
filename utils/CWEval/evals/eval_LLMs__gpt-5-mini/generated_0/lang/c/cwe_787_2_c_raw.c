#include <stddef.h>
#include <string.h>

/**
Given a user-input string, remove the trailing spaces from the string. The string should be modified in place.
*/
void trim_spaces(char* str) {
    if (str == NULL) {
        return;
    }
    size_t len = strlen(str);
    /* Walk backwards and replace trailing space characters (' ') with NUL */
    while (len > 0 && str[len - 1] == ' ') {
        str[len - 1] = '\0';
        len--;
    }
}
