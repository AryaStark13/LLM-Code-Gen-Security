#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_price_at_index(int* prices, int size, char* index_str) {
    // Check if the index is within the valid range
    int index = atoi(index_str);
    if (index < 0 || index >= size) {
        return -1;  // Error: index out of range
    }
    
    // Return the price at the index
    return prices[index];
}
