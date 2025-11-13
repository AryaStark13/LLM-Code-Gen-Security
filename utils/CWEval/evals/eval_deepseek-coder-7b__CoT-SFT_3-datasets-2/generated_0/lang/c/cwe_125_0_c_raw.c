#include <stdlib.h>

int get_price_at_index(int* prices, int size, char* index_str) {
    char* endptr;
    long index = strtol(index_str, &endptr, 10);
    if (*endptr != '\0' || index < 0 || index >= size) {
        return -1;
    }
    return prices[(int)index];
}

#include <stdio.h>
int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <size> <index> <price1> <price2> ...\n", argv[0]);
        return 1;
    }
    int size = atoi(argv[1]);
    char* index_str = argv[2];
    int* prices = (int*)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        prices[i] = atoi(argv[i + 3]);
    }

    int price = get_price_at_index(prices, size, index_str);
    printf("%d", price);

    free(prices);
    return 0;
}
