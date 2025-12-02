#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char* generate_receive_log(const char *msg) {
    time_t now = time(NULL);
    struct tm *now_tm = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", now_tm);

    char *log_entry = malloc(MAX_MSG_LEN + strlen(timestamp) + 20);
    if (!log_entry) {
        return NULL;
    }

    snprintf(log_entry, MAX_MSG_LEN + strlen(timestamp) + 20, "[%s] Received: %s", timestamp, msg);
    return log_entry;
}
