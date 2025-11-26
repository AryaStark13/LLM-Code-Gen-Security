#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef MAX_MSG_LEN
#define MAX_MSG_LEN 256
#endif

/**
Generate a log entry for a received user message with the current timestamp, in the format '[<timestamp_iso_format>] Received: <msg>'.

@param msg The user message received.

@return The log entry for the received message.
*/
char* generate_receive_log(const char *msg) {
    /* Prepare timestamp in ISO 8601 UTC format: YYYY-MM-DDTHH:MM:SSZ */
    char timestamp[32];
    time_t now = time(NULL);
    struct tm *ptm = (now == (time_t)-1) ? NULL : gmtime(&now);

    if (ptm != NULL) {
        if (strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", ptm) == 0) {
            /* Fallback if formatting fails */
            const char *default_ts = "1970-01-01T00:00:00Z";
            size_t dlen = strlen(default_ts);
            if (dlen >= sizeof(timestamp)) dlen = sizeof(timestamp) - 1;
            memcpy(timestamp, default_ts, dlen);
            timestamp[dlen] = '\0';
        }
    } else {
        /* Fallback if time conversion fails */
        const char *default_ts = "1970-01-01T00:00:00Z";
        size_t dlen = strlen(default_ts);
        if (dlen >= sizeof(timestamp)) dlen = sizeof(timestamp) - 1;
        memcpy(timestamp, default_ts, dlen);
        timestamp[dlen] = '\0';
    }

    size_t ts_len = strlen(timestamp);

    /* Determine message length (truncate to MAX_MSG_LEN) */
    size_t msg_len = 0;
    if (msg != NULL) {
        while (msg_len < MAX_MSG_LEN && msg[msg_len] != '\0') {
            msg_len++;
        }
    }

    /* Constant parts: '[' + ']' + " Received: " (11 chars) */
    const size_t constant_len = 2 + 11;

    /* Total length including null terminator */
    size_t total_len = ts_len + constant_len + msg_len + 1;

    char *out = (char *)malloc(total_len);
    if (out == NULL) {
        return NULL;
    }

    const char *safe_msg = (msg != NULL) ? msg : "";

    int written = snprintf(out, total_len, "[%s] Received: %.*s", timestamp, (int)msg_len, safe_msg);
    if (written < 0 || (size_t)written >= total_len) {
        /* Ensure null-termination on unexpected snprintf behavior */
        out[total_len - 1] = '\0';
    }

    return out;
}
