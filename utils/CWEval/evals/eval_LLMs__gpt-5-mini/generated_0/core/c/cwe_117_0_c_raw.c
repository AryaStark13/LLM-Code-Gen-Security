#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#define MAX_MSG_LEN 256

/**
Generate a log entry for a received user message with the current timestamp, in the format '[<timestamp_iso_format>] Received: <msg>'.

@param msg The user message received.

@return The log entry for the received message.
*/
char* generate_receive_log(const char *msg) {
    /* Treat NULL msg as empty string */
    if (msg == NULL) {
        msg = "";
    }

    /* Truncate message to MAX_MSG_LEN */
    size_t msg_len = strlen(msg);
    size_t copy_len = msg_len;
    if (copy_len > MAX_MSG_LEN) {
        copy_len = MAX_MSG_LEN;
    }

    char msg_buf[MAX_MSG_LEN + 1];
    if (copy_len > 0) {
        memcpy(msg_buf, msg, copy_len);
    }
    msg_buf[copy_len] = '\0';

    /* Get current time in UTC and format as ISO8601: YYYY-MM-DDTHH:MM:SSZ */
    time_t now = time(NULL);
    struct tm tm_snapshot;
    struct tm *tm_ptr = NULL;
#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) || defined(_BSD_SOURCE) || defined(_SVID_SOURCE) || defined(__APPLE__)
    /* If gmtime_r is available, use it for thread safety */
    if (gmtime_r(&now, &tm_snapshot) != NULL) {
        tm_ptr = &tm_snapshot;
    } else {
        tm_ptr = gmtime(&now);
    }
#else
    /* Fallback to gmtime (may not be thread-safe) */
    tm_ptr = gmtime(&now);
#endif

    char time_buf[32];
    if (tm_ptr != NULL) {
        if (strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ", tm_ptr) == 0) {
            /* Formatting failed; use empty timestamp */
            time_buf[0] = '\0';
        }
    } else {
        /* If time functions failed, use empty timestamp */
        time_buf[0] = '\0';
    }

    /* Compute required size for the final string */
    int needed = snprintf(NULL, 0, "[%s] Received: %s", time_buf, msg_buf);
    if (needed < 0) {
        /* snprintf encoding error; return empty string allocation */
        char *empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    /* Allocate buffer (needed + 1 for null terminator) */
    char *result = (char*)malloc((size_t)needed + 1);
    if (result == NULL) {
        /* Allocation failed; return empty string allocation to be graceful */
        char *empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    /* Write the formatted string into the allocated buffer */
    int written = snprintf(result, (size_t)needed + 1, "[%s] Received: %s", time_buf, msg_buf);
    if (written < 0) {
        /* Formatting error after allocation; ensure we return a valid string */
        result[0] = '\0';
    }

    return result;
}
