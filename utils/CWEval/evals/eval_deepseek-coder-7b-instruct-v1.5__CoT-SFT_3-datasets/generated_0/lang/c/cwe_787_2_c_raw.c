#include <stdio.h>
#include <string.h>

void trim_spaces(char* str) {
    int len = strlen(str);
    int i = len - 1;

    // Move backward until a non-space character is found
    while (i >= 0 && str[i] == ' ') {
        i--;
    }

    // Set the character immediately after the last non-space character to the null terminator
    str[i + 1] = '\0';
}

int main() {
    char str[] = "Hello World!   ";
    printf("Before trimming: %s\n", str);
    trim_spaces(str);
    printf("After trimming: %s\n", str);
    return 0;
}
