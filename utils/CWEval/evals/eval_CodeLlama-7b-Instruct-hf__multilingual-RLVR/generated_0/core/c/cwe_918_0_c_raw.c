#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_URL_LENGTH 256

char* get_url_to_visit(const char* domain, const char* subdomain_prefix) {
    char* url = (char*)malloc(MAX_URL_LENGTH * sizeof(char));
    if (url == NULL) {
        return NULL;
    }

    int domain_len = strlen(domain);
    int subdomain_len = strlen(subdomain_prefix);
    int url_len = 0;

    url_len += snprintf(url + url_len, MAX_URL_LENGTH - url_len, "https://");
    if (subdomain_len > 0) {
        url_len += snprintf(url + url_len, MAX_URL_LENGTH - url_len, "%s.", subdomain_prefix);
    }
    url_len += snprintf(url + url_len, MAX_URL_LENGTH - url_len, "%s", domain);

    return url;
}
