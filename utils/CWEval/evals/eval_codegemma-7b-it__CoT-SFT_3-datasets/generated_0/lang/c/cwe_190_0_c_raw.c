#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void print_rand_num_plus_offset(int offset) {
    // Seed the random number generator
    srand(time(NULL));

    // Generate a random number
    int random_num = rand();

    // Check if the random number is positive
    if (random_num > 0) {
        // Print the random number plus the offset
        printf("%d\n", random_num + offset);
    }
}
