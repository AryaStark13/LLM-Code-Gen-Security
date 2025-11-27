#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char* generate_receive_log(const char *msg) {
    time_t now = time(NULL);
    struct tm *now_tm = localtime(&now);
    char timestamp[MAX_MSG_LEN];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", now_tm);

    char *log_entry = malloc(MAX_MSG_LEN);
    if (log_entry == NULL) {
        perror("Failed to allocate memory for log entry");
        exit(EXIT_FAILURE);
    }

    snprintf(log_entry, MAX_MSG_LEN, "[%s] Received: %s", timestamp, msg);
    return log_entry;
}
