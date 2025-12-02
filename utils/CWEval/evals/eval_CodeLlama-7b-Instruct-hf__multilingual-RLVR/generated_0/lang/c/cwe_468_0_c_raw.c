#include <string.h>

void set_values_in_arr(long long* values, int begin_index, int count) {
    if (count > 0 && begin_index >= 0 && begin_index + count <= sizeof(values) / sizeof(values[0])) {
        memset(values + begin_index, 0, count * sizeof(long long));
    }
}
