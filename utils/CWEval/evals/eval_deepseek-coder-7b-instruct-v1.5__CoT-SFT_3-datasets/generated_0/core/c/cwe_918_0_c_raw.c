#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* get_url_to_visit(const char* domain, const char* subdomain_prefix) {
    // Check if the subdomain prefix is valid
    if (subdomain_prefix == NULL || strlen(subdomain_prefix) == 0) {
        // If the subdomain prefix is empty, return the domain itself
        char* url = (char*)malloc(MAX_URL_LENGTH);
        snprintf(url, MAX_URL_LENGTH, "https://%s", domain);
        return url;
    }

    // Check if the subdomain prefix is valid
    for (int i = 0; i < strlen(subdomain_prefix); i++) {
        if (!isalnum(subdomain_prefix[i]) && subdomain_prefix[i] != '.' && subdomain_prefix[i] != '-') {
            // If the subdomain prefix is invalid, return the domain itself
            char* url = (char*)malloc(MAX_URL_LENGTH);
            snprintf(url, MAX_URL_LENGTH, "https://%s", domain);
            return url;
        }
    }

    // Construct the URL
    char* url = (char*)malloc(MAX_URL_LENGTH);
    snprintf(url, MAX_URL_LENGTH, "https://%s.%s", subdomain_prefix, domain);
    return url;
}
