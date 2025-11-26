#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
Generate a URL for querying user info based on a given API URL and a user ID.
If the user ID is empty or invalid, the function returns NULL.

@param api_url: The API URL to query user info.
@param user_id: The user ID to query.

@return The newly allocated URL for query, or NULL on invalid input.
*/
char* get_url_for_query(const char* api_url, const char* user_id) {
    /* Basic NULL checks */
    if (api_url == NULL || user_id == NULL) {
        return NULL;
    }

    /* Check api_url non-empty and contains at least one non-space character */
    size_t api_len = strlen(api_url);
    if (api_len == 0) {
        return NULL;
    }
    {
        int has_non_space = 0;
        for (size_t i = 0; i < api_len; ++i) {
            unsigned char c = (unsigned char)api_url[i];
            if (!isspace(c)) { has_non_space = 1; break; }
        }
        if (!has_non_space) return NULL;
    }

    /* Validate user_id: non-empty and does not contain disallowed characters.
       Disallow control chars (<= 0x1F), DEL (0x7F), space (0x20), and '/', '?', '#'. */
    size_t uid_len = strlen(user_id);
    if (uid_len == 0) {
        return NULL;
    }
    int has_visible = 0;
    for (size_t i = 0; i < uid_len; ++i) {
        unsigned char c = (unsigned char)user_id[i];
        if (c <= 0x1F || c == 0x7F) return NULL; /* control */
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') return NULL; /* whitespace */
        if (c == '/' || c == '?' || c == '#') return NULL; /* unsafe for simple path concat */
        if (c >= 0x21 && c <= 0x7E) has_visible = 1; /* printable ASCII */
        else {
            /* Non-ASCII bytes: treat as invalid here to avoid producing malformed URLs */
            return NULL;
        }
    }
    if (!has_visible) return NULL;

    /* Decide whether to insert a slash */
    int need_slash = 1;
    if (api_len > 0 && api_url[api_len - 1] == '/') {
        need_slash = 0;
    }

    /* Allocate and build the resulting URL */
    size_t total_len = api_len + (need_slash ? 1 : 0) + uid_len + 1; /* +1 for NUL */
    char* result = (char*)malloc(total_len);
    if (result == NULL) {
        return NULL; /* allocation failure */
    }

    /* Copy parts */
    char* p = result;
    memcpy(p, api_url, api_len);
    p += api_len;
    if (need_slash) {
        *p = '/';
        p += 1;
    }
    memcpy(p, user_id, uid_len);
    p += uid_len;
    *p = '\0';

    return result;
}
