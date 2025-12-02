#include <string.h>

void trim_spaces(char* str) {
    int len = strlen(str);
    // Move the pointer to the end of the string
    while (len > 0 && str[len - 1] == ' ') {
        len--;
    }
    // Null terminate the string
    str[len] = '\0';
}
