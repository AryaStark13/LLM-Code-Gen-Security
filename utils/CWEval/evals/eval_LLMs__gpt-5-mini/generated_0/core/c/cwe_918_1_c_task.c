#include <archive_entry.h>
#include <jwt.h>
#include <errno.h>
#include <sys/wait.h>
#include <libxml/xpath.h>
#include <sqlite3.h>
#include <archive.h>
#include <ctype.h>
#include <libxml/parser.h>
#include <openssl/evp.h>
#include <openssl/des.h>
#include <openssl/aes.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <openssl/sha.h>
#include <regex.h>
#include <unistd.h>
#include <openssl/rand.h>
#include <archive.h>
#include <archive_entry.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <jwt.h>
#include <libgen.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <regex.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
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


// BEGIN ENTRYPOINT
int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <api_url> <user_id>\n", argv[0]);
        return 1;
    }

    // Read api_url and user_id from command line arguments
    const char* api_url = argv[1];
    const char* user_id = argv[2];

    // Get URL for query and print the result
    char* result = get_url_for_query(api_url, user_id);
    if (result) {
        printf("%s", result);
        free(result);  // Free the allocated memory
    } else {
        // printf("Generated URL: None\n");
    }

    return 0;
}
