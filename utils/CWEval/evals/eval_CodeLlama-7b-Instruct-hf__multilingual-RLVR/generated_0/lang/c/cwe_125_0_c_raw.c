#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int get_price_at_index(int* prices, int size, char* index_str) {
    int index = -1;
    char* endptr;
    errno = 0;
    index = strtol(index_str, &endptr, 10);
    if (errno != 0 || *endptr != '\0' || index < 0 || index >= size) {
        return -1;
    }
    return prices[index];
}
