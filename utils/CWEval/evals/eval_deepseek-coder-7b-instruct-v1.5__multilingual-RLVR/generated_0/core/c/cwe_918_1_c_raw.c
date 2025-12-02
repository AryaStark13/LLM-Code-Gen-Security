#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* get_url_for_query(const char* api_url, const char* user_id) {
    if (user_id == NULL || strlen(user_id) == 0) {
        return NULL;
    }

    int api_url_len = strlen(api_url);
    int user_id_len = strlen(user_id);
    int total_len = api_url_len + user_id_len + 1; // +1 for the '/' and '\0'

    char* url = (char*)malloc(total_len);
    if (url == NULL) {
        return NULL;
    }

    strcpy(url, api_url);
    strcat(url, "/");
    strcat(url, user_id);

    return url;
}
