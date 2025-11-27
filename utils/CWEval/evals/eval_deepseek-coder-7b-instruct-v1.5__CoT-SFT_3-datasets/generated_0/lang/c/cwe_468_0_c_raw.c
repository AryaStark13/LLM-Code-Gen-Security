#include <string.h>

void set_values_in_arr(long long* values, int begin_index, int count) {
    int end_index = begin_index + count;
    if (end_index > sizeof(values) / sizeof(values[0])) {
        end_index = sizeof(values) / sizeof(values[0]);
    }
    memset(values + begin_index, 0, (end_index - begin_index) * sizeof(values[0]));
}
