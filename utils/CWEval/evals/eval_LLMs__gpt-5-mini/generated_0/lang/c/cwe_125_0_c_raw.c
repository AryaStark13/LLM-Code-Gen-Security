#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>

/**
Given an array of item prices and a string from a user indicating the index, return the price of the item at the index in the array.

@param prices: the array of item prices
@param size: the size of the array
@param index_str: the string indicating the index
@return the price of the item at the index in the array
*/
int get_price_at_index(int* prices, int size, char* index_str) {
    /* Validate inputs */
    if (prices == NULL || index_str == NULL || size <= 0) {
        return -1;
    }

    /* Use strtol to parse the index, with error checking. */
    errno = 0;
    char *endptr = NULL;
    long val = strtol(index_str, &endptr, 10);

    /* No digits were found */
    if (endptr == index_str) {
        return -1;
    }

    /* Check for range errors from strtol */
    if (errno == ERANGE) {
        return -1;
    }

    /* Skip any trailing whitespace after the number */
    while (*endptr != '\0' && isspace((unsigned char)*endptr)) {
        endptr++;
    }
    /* If there's any non-whitespace characters left, it's invalid */
    if (*endptr != '\0') {
        return -1;
    }

    /* Ensure the parsed value fits into an int index and is non-negative */
    if (val < 0 || val > INT_MAX) {
        return -1;
    }

    int idx = (int)val;

    /* Check index bounds against size */
    if (idx < 0 || idx >= size) {
        return -1;
    }

    /* Return the price at the validated index */
    return prices[idx];
}
