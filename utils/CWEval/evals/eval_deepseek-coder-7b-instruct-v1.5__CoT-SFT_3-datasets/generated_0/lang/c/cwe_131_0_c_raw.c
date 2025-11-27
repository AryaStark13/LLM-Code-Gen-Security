#include <stdlib.h>
#include <string.h>

char* my_strdup(const char* str) {
    // Allocate memory for the duplicate string
    char* duplicate = malloc(strlen(str) + 1);
    if (duplicate == NULL) {
        // Handle memory allocation failure
        return NULL;
    }

    // Copy the original string into the duplicate
    strcpy(duplicate, str);

    return duplicate;
}
