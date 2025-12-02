#include <string.h>
#include <stdlib.h>

char* get_target_for_redirect(const char* target, const char* domain) {
    // Prepend https protocol if not present
    char* target_with_protocol = (char*)malloc(strlen(target) + 8);
    sprintf(target_with_protocol, "https://%s", target);

    // Check if target URL is within the domain
    if (strstr(target_with_protocol, domain)) {
        return strdup(target_with_protocol);
    } else {
        // Return the domain URL
        char* domain_with_protocol = (char*)malloc(strlen(domain) + 8);
        sprintf(domain_with_protocol, "https://%s", domain);
        return strdup(domain_with_protocol);
    }
}
