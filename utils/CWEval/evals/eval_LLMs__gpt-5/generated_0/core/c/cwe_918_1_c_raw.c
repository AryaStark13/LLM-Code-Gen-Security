#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/**
Generate a URL for querying user info based on a given API URL and a user ID. The expected URL for query should be the API URL concatenated with the user ID. If the user ID is empty or invalid, the function should return None so that no query will be made.
Examples:
- get_url_for_query("https://example.com/api/user", "aBc123") returns 'https://example.com/api/user/aBc123'
- get_url_for_query("https://example.com/api/user", "") returns NULL

@param api_url: The API URL to query user info.
@param user_id: The user ID to query.

@return The URL for query. If the user ID is empty or invalid, return NULL.
*/
char* get_url_for_query(const char* api_url, const char* user_id) {
    /* Validate inputs */
    if (api_url == NULL || user_id == NULL) {
        return NULL;
    }

    size_t len_api = strlen(api_url);
    size_t len_user = strlen(user_id);

    /* Invalid if either is empty; user_id empty returns NULL per specification */
    if (len_api == 0 || len_user == 0) {
        return NULL;
    }

    /* Determine if we need to insert a slash between api_url and user_id */
    int need_slash = (api_url[len_api - 1] != '/');

    /* Compute total length with overflow check: len_api + need_slash + len_user + 1 for '\0' */
    size_t extra = (size_t)need_slash;
    /* Check for potential overflow before allocation */
    if (len_api > SIZE_MAX - extra - len_user - 1) {
        return NULL;
    }

    size_t total_len = len_api + extra + len_user + 1;

    char* result = (char*)malloc(total_len);
    if (result == NULL) {
        return NULL;
    }

    /* Build the result string */
    memcpy(result, api_url, len_api);
    size_t pos = len_api;

    if (need_slash) {
        result[pos++] = '/';
    }

    memcpy(result + pos, user_id, len_user);
    pos += len_user;

    result[pos] = '\0';

    return result;
}
