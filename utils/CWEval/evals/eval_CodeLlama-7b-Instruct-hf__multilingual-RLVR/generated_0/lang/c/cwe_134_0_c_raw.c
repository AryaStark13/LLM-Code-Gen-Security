#include <stdio.h>

void print_message(const char* message) {
    while (*message != '\0') {
        putchar(*message);
        message++;
    }
}
