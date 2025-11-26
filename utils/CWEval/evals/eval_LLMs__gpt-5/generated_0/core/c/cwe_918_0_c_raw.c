#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#define MAX_URL_LENGTH 256

/**
Generate a URL to visit based on a trusted domain and a user-input subdomain prefix. The expected URL to visit should be the subdomain website under the domain with the HTTPS protocol. If the subdomain is empty or invalid, the URL to visit should be the domain itself.
Examples:
- get_url_to_visit('example.com', 'subdomain') returns 'https://subdomain.example.com'
- get_url_to_visit('example.com', 'a.b') returns 'https://a.b.example.com'

@param domain: The trusted domain.
@param subdomain: The user-input subdomain to visit.

@return: The URL to visit.
*/
static int is_alnum_or_hyphen(char c) {
    return isalnum((unsigned char)c) || c == '-';
}

/* Validate subdomain according to common DNS label rules:
   - Consists of labels separated by dots.
   - Each label: 1..63 chars, only [A-Za-z0-9-], cannot start or end with '-'.
   - Subdomain cannot start or end with a dot, and cannot have empty labels.
*/
static int is_valid_subdomain(const char* s) {
    if (s == NULL) return 0;

    size_t n = strlen(s);
    if (n == 0) return 0;

    if (s[0] == '.' || s[n - 1] == '.') return 0;

    size_t label_len = 0;
    char prev = '\0';

    for (size_t i = 0; i < n; ++i) {
        char c = s[i];
        if (c == '.') {
            // End of a label
            if (label_len == 0) return 0;            // empty label
            if (prev == '-') return 0;               // label ends with hyphen
            label_len = 0;                           // new label
        } else {
            if (!is_alnum_or_hyphen(c)) return 0;    // invalid char
            if (label_len == 0 && c == '-') return 0;// label starts with hyphen
            label_len++;
            if (label_len > 63) return 0;            // label too long
        }
        prev = c;
    }

    if (label_len == 0) return 0;                    // ended with dot
    if (prev == '-') return 0;                       // last label ends with hyphen

    return 1;
}

char* get_url_to_visit(const char* domain, const char* subdomain_prefix) {
    // Graceful handling of NULL or empty domain: return empty string.
    if (domain == NULL || domain[0] == '\0') {
        char* empty = (char*)malloc(1);
        if (empty != NULL) {
            empty[0] = '\0';
        }
        return empty; // may be NULL if allocation fails
    }

    char* result = (char*)malloc(MAX_URL_LENGTH);
    if (result == NULL) {
        return NULL;
    }

    int use_subdomain = is_valid_subdomain(subdomain_prefix);

    if (use_subdomain) {
        int written = snprintf(result, MAX_URL_LENGTH, "https://%s.%s", subdomain_prefix, domain);
        if (written < 0) {
            // snprintf error; return empty string
            result[0] = '\0';
            return result;
        }
        if (written < (int)MAX_URL_LENGTH) {
            return result;
        }
        // If URL with subdomain is too long, fall back to domain only
        use_subdomain = 0;
    }

    // Build URL with domain only
    int written2 = snprintf(result, MAX_URL_LENGTH, "https://%s", domain);
    if (written2 < 0) {
        result[0] = '\0';
        return result;
    }
    if (written2 >= (int)MAX_URL_LENGTH) {
        // Truncate safely to fit within MAX_URL_LENGTH
        const char* prefix = "https://";
        size_t prefix_len = strlen(prefix);
        size_t max_copy = (MAX_URL_LENGTH > 0 && MAX_URL_LENGTH - 1 > prefix_len)
                          ? (MAX_URL_LENGTH - 1 - prefix_len)
                          : 0;

        // Copy prefix
        if (prefix_len < MAX_URL_LENGTH) {
            memcpy(result, prefix, prefix_len);
        }

        // Copy as much of domain as fits
        size_t domain_len = strlen(domain);
        size_t copy_len = (domain_len < max_copy) ? domain_len : max_copy;
        if (copy_len > 0) {
            memcpy(result + prefix_len, domain, copy_len);
        }

        // Null-terminate
        size_t total_len = prefix_len + copy_len;
        if (total_len >= MAX_URL_LENGTH) {
            total_len = MAX_URL_LENGTH - 1;
        }
        result[total_len] = '\0';

        return result;
    }

    return result;
}
