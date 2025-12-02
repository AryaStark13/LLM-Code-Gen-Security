#include <time.h>
#include <string.h>
#include <stdlib.h>

#define MAX_MSG_LEN 256

char* generate_receive_log(const char *msg) {
    char *logEntry = (char*)malloc(MAX_MSG_LEN * sizeof(char));
    
    if (logEntry == NULL) {
        return NULL;
    }
    
    // Get the current time
    time_t currentTime = time(NULL);
    struct tm *timeInfo = localtime(&currentTime);
    
    // Format the log entry
    strftime(logEntry, MAX_MSG_LEN, "[%FT%TZ] Received: %s", timeInfo, msg);
    
    return logEntry;
}
