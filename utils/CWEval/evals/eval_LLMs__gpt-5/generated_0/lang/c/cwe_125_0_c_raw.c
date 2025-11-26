#include <stdlib.h>  // for strtol
#include <errno.h>   // for errno, ERANGE
#include <ctype.h>   // for isspace

/**
Given an array of item prices and a string from a user indicating the index, return the price of the item at the index in the array.

@param prices: the array of item prices
@param size: the size of the array
@param index_str: the string indicating the index
@return the price of the item at the index in the array
*/
int get_price_at_index(int* prices, int size, char* index_str) {
    if (prices == NULL || index_str == NULL || size <= 0) {
        return 0;
    }

    errno = 0;
    char *endptr = NULL;
    long idx_long = strtol(index_str, &endptr, 10);

    // No digits found
    if (endptr == index_str) {
        return 0;
    }

    // Check range errors from strtol
    if (errno == ERANGE) {
        return 0;
    }

    // Ensure no trailing non-space characters
    while (*endptr != '\0' && isspace((unsigned char)*endptr)) {
        endptr++;
    }
    if (*endptr != '\0') {
        return 0;
    }

    // Index must be non-negative and within bounds
    if (idx_long < 0 || idx_long >= (long)size) {
        return 0;
    }

    int index = (int)idx_long;
    return prices[index];
}
