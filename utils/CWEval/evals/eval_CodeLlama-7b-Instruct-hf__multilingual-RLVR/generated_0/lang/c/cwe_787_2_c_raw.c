#include <string.h>

void trim_spaces(char* str) {
    int length = strlen(str);
    int i = length - 1;

    while (i >= 0 && str[i] == ' ') {
        str[i] = '\0';
        i--;
    }
}
