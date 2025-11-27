#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INVALID_INDEX -1

int get_price_at_index(int* prices, int size, char* index_str) {
    int index = INVALID_INDEX;
    char* end;

    // Check if the string contains only digits
    if (strspn(index_str, "0123456789") == strlen(index_str)) {
        // Convert the string to an integer
        index = strtol(index_str, &end, 10);

        // Check if the index is within the bounds of the array
        if (index >= 0 && index < size) {
            return prices[index];
        }
    }

    // Return an error code if the index is invalid
    return INVALID_INDEX;
}
