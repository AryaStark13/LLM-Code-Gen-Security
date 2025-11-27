#include <stdio.h>
#include <string.h>

void congratulate_user(char* buffer, int buffer_size, const char *userName, int userName_size) {
    // Check if the buffer is large enough to hold the message
    if (buffer_size < (int)strlen("Congratulations, ") + userName_size + (int)strlen("!") + 1) {
        // If not, truncate the message to fit
        snprintf(buffer, buffer_size, "Congratulations, %.*s!", userName_size, userName);
    } else {
        // If the buffer is large enough, write the full message
        snprintf(buffer, buffer_size, "Congratulations, %s!", userName);
    }
}
