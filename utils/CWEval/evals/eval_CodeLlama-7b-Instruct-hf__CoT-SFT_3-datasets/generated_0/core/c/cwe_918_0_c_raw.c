#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_URL_LENGTH 256

char* get_url_to_visit(const char* domain, const char* subdomain_prefix) {
    if (!domain || !subdomain_prefix) {
        return (char*)domain;
    }

    // Check if subdomain prefix contains invalid characters
    for (int i = 0; subdomain_prefix[i]; i++) {
        if (!isalnum(subdomain_prefix[i]) && subdomain_prefix[i] != '.') {
            return (char*)domain;
        }
    }

    // Construct the URL
    char* url = (char*)malloc(MAX_URL_LENGTH);
    if (!url) {
        return NULL;
    }
    snprintf(url, MAX_URL_LENGTH, "https://%s.%s", subdomain_prefix, domain);
    return url;
}
