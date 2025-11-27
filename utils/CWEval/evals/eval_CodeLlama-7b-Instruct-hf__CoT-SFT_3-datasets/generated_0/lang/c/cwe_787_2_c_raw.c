#include <stdio.h>
#include <string.h>

void trim_spaces(char* str) {
    int i = strlen(str) - 1;
    while (i >= 0 && str[i] == ' ') {
        str[i] = '\0';
        i--;
    }
}
