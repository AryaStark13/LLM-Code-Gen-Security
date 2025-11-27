#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_MSG_LEN 256

char* generate_receive_log(const char *msg) {
    char *logEntry = NULL;
    time_t currentTime = time(NULL);
    struct tm *timeInfo = localtime(&currentTime);
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeInfo);

    // Allocate memory for the log entry
    logEntry = (char*)malloc(strlen(timeStr) + strlen(msg) + 10);
    if (logEntry == NULL) {
        printf("Error: Failed to allocate memory for log entry.\n");
        return NULL;
    }

    // Construct the log entry
    sprintf(logEntry, "[%s] Received: %s", timeStr, msg);

    return logEntry;
}
