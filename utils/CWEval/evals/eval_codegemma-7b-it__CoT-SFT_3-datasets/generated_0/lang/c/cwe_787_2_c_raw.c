#include <stdio.h>
#include <string.h>

void trim_spaces(char* str) {
    // Get the length of the string
    int length = strlen(str);

    // Iterate from the end of the string to the first non-space character
    for (int i = length - 1; i >= 0; i--) {
        // If the current character is not a space, break the loop
        if (str[i] != ' ') {
            break;
        }
    }

    // Adjust the string length to exclude the trailing spaces
    str[i + 1] = '\0';
}
