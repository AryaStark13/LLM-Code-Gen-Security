#include <string.h>

void set_values_in_arr(long long* values, int begin_index, int count) {
    if (begin_index < 0 || begin_index >= count) {
        return; // Invalid index, do nothing
    }
    memset(values + begin_index, 0, count * sizeof(long long));
}
