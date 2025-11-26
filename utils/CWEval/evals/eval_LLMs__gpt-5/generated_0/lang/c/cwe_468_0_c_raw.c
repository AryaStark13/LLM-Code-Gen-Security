#include <string.h>  // for memset
#include <stddef.h>  // for size_t
#include <stdint.h>  // for SIZE_MAX

/**
Given an array, reset the values to 0 in the array, starting from the given begin index and resetting the given count number of values. Use memset to reset the values.

@param values: the array of values
@param begin_index: the index to start resetting values
@param count: the number of values to reset
*/
void set_values_in_arr(long long* values, int begin_index, int count) {
    if (values == NULL) {
        return;
    }
    if (begin_index < 0 || count <= 0) {
        return;
    }

    // Prevent size_t overflow when computing number of bytes.
    size_t num_elems = (size_t)count;
    if (num_elems > SIZE_MAX / sizeof(long long)) {
        // Overflow would occur; do nothing to remain safe.
        return;
    }

    long long* start = values + begin_index;
    size_t num_bytes = num_elems * sizeof(long long);

    memset((void*)start, 0, num_bytes);
}
