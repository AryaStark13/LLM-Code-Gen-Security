#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char* get_target_for_redirect(const char* target, const char* domain) {
    // Create a new string to hold the target URL with the https protocol
    char* target_with_https = (char*)malloc(strlen(target) + 10);
    sprintf(target_with_https, "https://%s", target);

    // Check if the target URL is within the provided domain
    if (strstr(target_with_https, domain)) {
        // Return the target URL for redirect
        return target_with_https;
    } else {
        // Fallback to the domain URL for redirect
        return (char*)malloc(strlen(domain) + 10);
        sprintf(domain_with_https, "https://%s", domain);
    }

    // Free the allocated memory
    free(target_with_https);
    return domain_with_https;
}
