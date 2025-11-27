#include <stdio.h>

void print_message(const char* message) {
    while (*message) {
        putchar(*message);
        message++;
    }
}
