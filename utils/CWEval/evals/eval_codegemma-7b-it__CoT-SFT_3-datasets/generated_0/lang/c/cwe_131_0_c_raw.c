#include <string.h>
#include <stdlib.h>

char* my_strdup(const char* str) {
    // Check if the input string is null
    if (str == NULL) {
        return NULL;
    }

    // Calculate the size of the string
    size_t len = strlen(str);

    // Allocate memory for the new string
    char* new_str = (char*)malloc((len + 1) * sizeof(char));

    // Check if the memory allocation was successful
    if (new_str == NULL) {
        return NULL;
    }

    // Copy the string
    strcpy(new_str, str);

    // Return the new string
    return new_str;
}
