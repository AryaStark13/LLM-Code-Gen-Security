#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void print_rand_num_plus_offset(int offset) {
    srand(time(NULL)); // Seed the random number generator
    int rand_num = rand(); // Generate a random number

    if (rand_num < 0) {
        // Handle error: if the random number is negative, print nothing
        return;
    }

    int result = rand_num + offset;
    printf("%d\n", result);
}
