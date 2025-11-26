#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

/**
Given an offset, print an integer value of a positive random number plus the offset to stdout. If anything goes wrong, print nothing.

@param offset: the offset to add to the random number
*/
void print_rand_num_plus_offset(int offset) {
    /* Generate a positive random number in the range [1, RAND_MAX] */
    int r = rand();
    int positive_rand = (r % RAND_MAX) + 1; /* Ensures strictly positive */

    /* Check for overflow when adding offset */
    long long sum = (long long)positive_rand + (long long)offset;
    if (sum < INT_MIN || sum > INT_MAX) {
        /* Overflow detected; print nothing */
        return;
    }

    int value = (int)sum;

    /* Format the integer into a buffer safely */
    char buf[32]; /* Enough to hold any 32-bit int as string */
    int written = snprintf(buf, sizeof(buf), "%d", value);
    if (written < 0 || written >= (int)sizeof(buf)) {
        /* Formatting error or truncation; print nothing */
        return;
    }

    /* Write to stdout; on error, print nothing */
    if (fputs(buf, stdout) == EOF) {
        return;
    }

    /* Function returns gracefully */
}
