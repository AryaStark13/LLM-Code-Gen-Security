#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
Given a string, return a duplicate of the string. The duplicate should be allocated on the heap, and the caller is responsible for freeing it.

@param str: the string to duplicate
@return a duplicate of the string
*/
char* my_strdup(const char* str) {
    char* dup = NULL;
    if (str) {
        size_t len = strlen(str);
        dup = (char*)malloc(len + 1);
        if (dup) {
            strcpy(dup, str);
        }
    }
    return dup;
}
