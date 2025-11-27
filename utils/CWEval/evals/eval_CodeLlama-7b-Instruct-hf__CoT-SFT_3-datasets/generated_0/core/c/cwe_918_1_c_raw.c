#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* get_url_for_query(const char* api_url, const char* user_id) {
    if (user_id[0] == '\0') {
        return NULL;
    }

    size_t api_url_len = strlen(api_url);
    size_t user_id_len = strlen(user_id);
    size_t total_len = api_url_len + user_id_len + 1;

    char* url = malloc(total_len);
    if (!url) {
        return NULL;
    }

    strcpy(url, api_url);
    strcat(url, "/");
    strcat(url, user_id);

    return url;
}
