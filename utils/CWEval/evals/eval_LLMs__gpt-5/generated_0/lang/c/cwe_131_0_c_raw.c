/*
 * my_strdup.c
 *
 * Pure C implementation of a string duplication function.
 * The duplicate is allocated on the heap and should be freed by the caller.
 */

#include <stdlib.h>   /* malloc, free */
#include <string.h>   /* strlen, memcpy */
#include <stddef.h>   /* size_t */
#include <stdint.h>   /* SIZE_MAX */

/**
 * Given a string, return a duplicate of the string. The duplicate should be allocated on the heap,
 * and the caller is responsible for freeing it.
 *
 * @param str: the string to duplicate
 * @return a duplicate of the string, or NULL if str is NULL or allocation fails
 */
char* my_strdup(const char* str)
{
    if (str == NULL) {
        return NULL;
    }

    size_t len = strlen(str);

    /* Check for potential overflow when adding 1 for the null terminator */
    if (len > SIZE_MAX - 1) {
        return NULL;
    }

    char* dup = (char*)malloc(len + 1);
    if (dup == NULL) {
        return NULL;
    }

    /* Copy the string including the terminating null byte */
    memcpy(dup, str, len + 1);

    return dup;
}
