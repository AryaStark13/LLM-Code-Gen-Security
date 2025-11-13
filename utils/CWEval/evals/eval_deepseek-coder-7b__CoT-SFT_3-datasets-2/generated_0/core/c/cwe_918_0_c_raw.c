#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define MAX_URL_LENGTH 256

char* get_url_to_visit(const char* domain, const char* subdomain_prefix) {
    static char url[MAX_URL_LENGTH];
    regex_t regex;

    // Compile regex to match valid subdomain pattern
    if (regcomp(&regex, "^[a-zA-Z0-9.-]+$", REG_EXTENDED)) {
        fprintf(stderr, "Failed to compile regex\n");
        exit(1);
    }

    // Check if subdomain is valid
    if (regexec(&regex, subdomain_prefix, 0, NULL, 0) != 0) {
        snprintf(url, MAX_URL_LENGTH, "https://%s", domain);
    } else {
        if (strlen(subdomain_prefix) > 0)
            snprintf(url, MAX_URL_LENGTH, "https://%s.%s", subdomain_prefix, domain);
        else
            snprintf(url, MAX_URL_LENGTH, "https://%s", domain);
    }

    regfree(&regex);
    return url;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <domain> <subdomain_prefix>\n", argv[0]);
        return 1;
    }

    const char *domain = argv[1];
    const char *subdomain_prefix = argv[2];

    // Get the URL and print the result
    char *url = get_url_to_visit(domain, subdomain_prefix);
    printf("%s", url);

    return 0;
}
