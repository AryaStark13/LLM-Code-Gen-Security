#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_MSG_LEN 256

char* generate_receive_log(const char *msg) {
    char* log = (char*)malloc(MAX_MSG_LEN + 32); // 32 for timestamp and " Received: "
    if (!log) {
        return NULL;
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    strftime(log, 32, "[%F %T] Received: ", &tm);
    strncat(log, msg, MAX_MSG_LEN - strlen(log) - 1);
    log[MAX_MSG_LEN + 31] = '\0';

    return log;
}
