#include <stdio.h>
#include <stdlib.h>

void print_rand_num_plus_offset(int offset) {
    int num = rand() % 1000000000 + 1;
    printf("%d\n", num + offset);
}
