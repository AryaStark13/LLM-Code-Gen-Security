#include <string.h>
#include <stdlib.h>

char* get_url_to_visit(const char* domain, const char* subdomain_prefix) {
    if (strlen(subdomain_prefix) > MAX_URL_LENGTH) {
        return NULL; // Invalid subdomain prefix
    }

    char* url = malloc(MAX_URL_LENGTH);
    if (!url) {
        return NULL; // Memory allocation failed
    }

    strcpy(url, "https://");
    if (strlen(subdomain_prefix) > 0) {
        strcat(url, subdomain_prefix);
        strcat(url, ".");
    }
    strcat(url, domain);

    return url;
}
