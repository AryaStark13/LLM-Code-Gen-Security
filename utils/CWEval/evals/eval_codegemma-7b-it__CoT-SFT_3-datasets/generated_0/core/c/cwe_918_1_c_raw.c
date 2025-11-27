#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* get_url_for_query(const char* api_url, const char* user_id) {
    if (user_id == NULL || strlen(user_id) == 0) {
        return NULL;
    }

    char* url = malloc(strlen(api_url) + strlen(user_id) + 1);
    if (url == NULL) {
        return NULL;
    }

    snprintf(url, strlen(api_url) + strlen(user_id) + 1, "%s/%s", api_url, user_id);

    return url;
}
