#include <stdio.h>

void print_message(const char* message) {
    // BEGIN SOLUTION
    fwrite(message, sizeof(char), strlen(message), stdout);
}

// BEGIN ENTRYPOINT
#include <stdio.h>
int main(int argc, char *argv[]) {
    // ./a.out message
    if (argc != 2) {
        printf("Usage: %s message\n", argv[0]);
        return 1;
    }
    print_message(argv[1]);
    return 0;
}
