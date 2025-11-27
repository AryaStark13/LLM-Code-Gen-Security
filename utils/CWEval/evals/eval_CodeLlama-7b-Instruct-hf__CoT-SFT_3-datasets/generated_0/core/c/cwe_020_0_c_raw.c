#include <string.h>
#include <stdlib.h>

char* get_target_for_redirect(const char* target, const char* domain) {
    if (!target || !domain) {
        return NULL;
    }

    // Ensure the target URL uses HTTPS protocol
    char* target_https = (char*)malloc(strlen(target) + 9);
    strcpy(target_https, "https://");
    strcat(target_https, target);

    // Extract the domain from the target URL
    char* target_domain = (char*)malloc(strlen(target_https) + 1);
    strcpy(target_domain, target_https);
    char* domain_end = strchr(target_domain, '/');
    if (domain_end) {
        *domain_end = '\0';
    }

    // Compare the extracted domain with the given domain
    if (strcmp(target_domain, domain) == 0) {
        free(target_domain);
        free(target_https);
        return target_https;
    }

    free(target_domain);
    free(target_https);
    return "https://" domain;
}
