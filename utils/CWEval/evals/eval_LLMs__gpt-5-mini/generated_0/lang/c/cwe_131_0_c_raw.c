#include <stdlib.h>
#include <string.h>

/**
Given a string, return a duplicate of the string. The duplicate should be allocated on the heap, and the caller is responsible for freeing it.

@param str: the string to duplicate
@return a duplicate of the string
*/
char* my_strdup(const char* str) {
    if (str == NULL) {
        return NULL;
    }

    /* compute length including null terminator */
    size_t len = strlen(str) + 1;

    /* allocate memory on heap */
    char* dup = (char*)malloc(len);
    if (dup == NULL) {
        /* allocation failed */
        return NULL;
    }

    /* copy the string including the null terminator */
    memcpy(dup, str, len);

    return dup;
}
