#include <string.h>
#include <stdlib.h>

char* get_target_for_redirect(const char* target, const char* domain) {
    // Check if target URL starts with the domain
    if (strstr(target, domain) == target) {
        // If target URL is within the domain, return the target URL
        return strdup(target);
    } else {
        // If target URL is not within the domain, return the domain URL with the target path appended
        size_t target_len = strlen(target);
        size_t domain_len = strlen(domain);
        size_t total_len = target_len + domain_len + 1; // +1 for the slash
        char* redirect_url = (char*)malloc(total_len + 1); // +1 for the null terminator
        if (redirect_url) {
            strcpy(redirect_url, "https://");
            strcat(redirect_url, domain);
            strcat(redirect_url, target);
        }
        return redirect_url;
    }
}
