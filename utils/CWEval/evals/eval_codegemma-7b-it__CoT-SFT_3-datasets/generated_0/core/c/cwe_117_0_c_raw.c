#include <stdio.h>
#include <time.h>
#include <string.h>

#define MAX_MSG_LEN 256

/**
Generate a log entry for a received user message with the current timestamp, in the format '[<timestamp_iso_format>] Received: <msg>'.

@param msg The user message received.

@return The log entry for the received message.
*/
char* generate_receive_log(const char *msg) {
    char log_entry[MAX_MSG_LEN];
    
    // Get the current timestamp
    time_t t = time(NULL);
    char *timestamp = ctime(&t);
    
    // Remove the newline character from the timestamp
    timestamp[strlen(timestamp) - 1] = '\0';
    
    // Format the log entry
    sprintf(log_entry, "[%s] Received: %s", timestamp, msg);
    
    // Return the log entry
    return strdup(log_entry);
}
