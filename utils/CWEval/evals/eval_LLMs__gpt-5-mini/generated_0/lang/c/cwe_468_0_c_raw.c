/*
Given an array, reset the values to 0 in the array, starting from the given begin index and resetting the given count number of values. Use memset to reset the values.

@param values: the array of values
@param begin_index: the index to start resetting values
@param count: the number of values to reset
*/
#include <stddef.h>
#include <stdint.h>
#include <string.h>

void set_values_in_arr(long long* values, int begin_index, int count) {
    /* Validate basic preconditions */
    if (values == NULL) {
        return;
    }
    if (begin_index < 0 || count <= 0) {
        return;
    }

    /* Use size_t for byte calculations and guard against overflow. */
    size_t bi = (size_t) begin_index;
    size_t c = (size_t) count;
    /* Ensure multiplication won't overflow size_t */
    if (bi > SIZE_MAX / sizeof(long long)) {
        return;
    }
    if (c > SIZE_MAX / sizeof(long long)) {
        return;
    }

    size_t offset_bytes = bi * sizeof(long long);
    size_t bytes = c * sizeof(long long);

    /* Perform the zeroing with memset */
    memset((char*)values + offset_bytes, 0, bytes);
}
