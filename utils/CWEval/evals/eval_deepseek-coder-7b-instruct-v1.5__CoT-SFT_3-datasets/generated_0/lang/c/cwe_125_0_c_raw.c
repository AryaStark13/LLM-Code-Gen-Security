#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_price_at_index(int* prices, int size, char* index_str) {
    int index = atoi(index_str);
    if (index < 0 || index >= size) {
        return -1;
    }
    return prices[index];
}
