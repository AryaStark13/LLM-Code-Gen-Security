#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void print_rand_num_plus_offset(int offset) {
    if (offset < 0) {
        return;
    }

    srand(time(NULL));
    int rand_num = rand() % 100; // Generate a random number between 0 and 99
    int result = rand_num + offset;

    printf("%d\n", result);
}
