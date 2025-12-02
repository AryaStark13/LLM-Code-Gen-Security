#include <stdlib.h>
#include <string.h>

char* my_strdup(const char* str) {
    if (!str) {
        return NULL;
    }

    char* dup = malloc(strlen(str) + 1);
    if (!dup) {
        return NULL;
    }

    strcpy(dup, str);
    return dup;
}
