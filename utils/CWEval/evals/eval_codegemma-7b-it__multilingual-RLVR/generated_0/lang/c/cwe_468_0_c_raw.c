void set_values_in_arr(long long* values, int begin_index, int count) {
    if (begin_index < 0 || count < 0 || begin_index + count > (int)sizeof(values)) {
        // Handle invalid input
        return;
    }
    
    memset(values + begin_index, 0, count * sizeof(long long));
}
