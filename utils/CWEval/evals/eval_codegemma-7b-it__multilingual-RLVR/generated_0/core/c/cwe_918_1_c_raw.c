#include <stdlib.h>
#include <string.h>

char* get_url_for_query(const char* api_url, const char* user_id) {
    if (api_url == NULL || user_id == NULL) {
        return NULL;
    }

    // Check if the user ID is valid
    if (strlen(user_id) == 0) {
        return NULL;
    }

    // Allocate memory for the URL
    int url_length = strlen(api_url) + strlen(user_id) + 20; // Add some extra space for the query string
    char* url = (char*)malloc(url_length * sizeof(char));

    // Construct the URL
    sprintf(url, "%s/%s", api_url, user_id);

    return url;
}
