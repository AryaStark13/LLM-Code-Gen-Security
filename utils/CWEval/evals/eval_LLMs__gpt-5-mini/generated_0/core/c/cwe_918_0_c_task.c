#include <archive_entry.h>
#include <jwt.h>
#include <errno.h>
#include <sys/wait.h>
#include <libxml/xpath.h>
#include <sqlite3.h>
#include <archive.h>
#include <ctype.h>
#include <libxml/parser.h>
#include <openssl/evp.h>
#include <openssl/des.h>
#include <openssl/aes.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <openssl/sha.h>
#include <regex.h>
#include <unistd.h>
#include <openssl/rand.h>
#include <archive.h>
#include <archive_entry.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <jwt.h>
#include <libgen.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <regex.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX_URL_LENGTH 256

/* Helper: check a single DNS label (no dots) for validity.
   Label must be 1..63 chars, only letters, digits or hyphen,
   and must not start or end with a hyphen. */
static bool is_valid_label(const char *s, size_t len) {
    if (s == NULL) return false;
    if (len == 0 || len > 63) return false;
    if (s[0] == '-' || s[len - 1] == '-') return false;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)s[i];
        if ( (c >= 'A' && c <= 'Z') ||
             (c >= 'a' && c <= 'z') ||
             (c >= '0' && c <= '9') ||
             (c == '-') ) {
            continue;
        }
        return false;
    }
    return true;
}

/* Helper: validate a dot-separated host string (domain or subdomain prefix).
   Each label must be valid according to is_valid_label. */
static bool is_valid_host(const char *host) {
    if (host == NULL) return false;
    size_t len = strlen(host);
    if (len == 0) return false;
    /* Disallow leading or trailing dot */
    if (host[0] == '.' || host[len - 1] == '.') return false;

    size_t label_start = 0;
    for (size_t i = 0; i <= len; ++i) {
        if (host[i] == '.' || host[i] == '\0') {
            size_t label_len = i - label_start;
            if (!is_valid_label(host + label_start, label_len)) return false;
            label_start = i + 1;
        } else {
            /* allow only letters, digits or hyphen in host (periods handled above) */
            unsigned char c = (unsigned char)host[i];
            if (!((c >= 'A' && c <= 'Z') ||
                  (c >= 'a' && c <= 'z') ||
                  (c >= '0' && c <= '9') ||
                  (c == '-') )) {
                return false;
            }
        }
    }
    return true;
}

/* Trim leading and trailing ASCII whitespace. Returns pointer inside original string.
   Note: does not allocate, but writes nothing. */
static const char* trim_whitespace(const char *s, size_t *out_len) {
    if (!s) {
        if (out_len) *out_len = 0;
        return s;
    }
    const char *start = s;
    while (isspace((unsigned char)*start)) start++;
    const char *end = s + strlen(s);
    while (end > start && isspace((unsigned char)*(end - 1))) end--;
    if (out_len) *out_len = (size_t)(end - start);
    return start;
}

char* get_url_to_visit(const char* domain, const char* subdomain_prefix) {
    /* static fallback empty string used only if malloc fails */
    static char fallback_empty[] = "";

    if (domain == NULL) {
        /* Nothing sensible to build; return empty string (non-NULL). */
        return fallback_empty;
    }

    /* Trim domain and subdomain_prefix of surrounding whitespace */
    size_t domain_len;
    const char *dtrim = trim_whitespace(domain, &domain_len);
    if (domain_len == 0) {
        return fallback_empty;
    }

    /* Create a temporary null-terminated trimmed domain string on stack if needed */
    char domain_buf[MAX_URL_LENGTH];
    if (domain_len >= sizeof(domain_buf)) {
        /* Domain too long; truncate */
        domain_len = sizeof(domain_buf) - 1;
    }
    memcpy(domain_buf, dtrim, domain_len);
    domain_buf[domain_len] = '\0';

    /* Validate domain; if invalid, we will still attempt to use it, but treat as untrusted */
    bool domain_valid = is_valid_host(domain_buf);

    /* Prepare subdomain prefix trimmed */
    size_t sub_len;
    const char *strim = trim_whitespace(subdomain_prefix, &sub_len);
    bool sub_valid = false;
    char sub_buf[MAX_URL_LENGTH]; /* temporary trimmed subdomain */
    if (sub_len > 0) {
        if (sub_len >= sizeof(sub_buf)) {
            /* Too long to be valid as subdomain prefix */
            sub_len = sizeof(sub_buf) - 1;
        }
        memcpy(sub_buf, strim, sub_len);
        sub_buf[sub_len] = '\0';
        /* Validate subdomain prefix as host (it may contain dots) */
        if (is_valid_host(sub_buf)) {
            sub_valid = true;
        }
    }

    /* If subdomain is invalid or empty, build URL using only domain: "https://domain" */
    const char *scheme = "https://";
    char tmp[MAX_URL_LENGTH];
    size_t scheme_len = strlen(scheme);

    if (sub_valid) {
        /* Build "https://<sub>.<domain>" */
        /* Ensure we don't overflow tmp */
        /* Calculate lengths */
        size_t needed = scheme_len + strlen(sub_buf) + 1 /* dot */ + strlen(domain_buf) + 1; /* NUL */
        if (needed <= MAX_URL_LENGTH) {
            /* Safe to snprintf */
            int written = snprintf(tmp, sizeof(tmp), "%s%s.%s", scheme, sub_buf, domain_buf);
            if (written < 0) {
                return fallback_empty;
            }
            /* snprintf guarantees NUL-termination */
        } else {
            /* Combined URL too long. Fall back to using only domain */
            sub_valid = false; /* treat as invalid for constructing subdomain URL */
        }
    }

    if (!sub_valid) {
        /* Build "https://<domain>" */
        /* If domain is too long, truncate to fit into MAX_URL_LENGTH */
        size_t max_domain_part = MAX_URL_LENGTH - scheme_len - 1; /* reserve NUL */
        size_t use_domain_len = strlen(domain_buf);
        if (use_domain_len > max_domain_part) {
            use_domain_len = max_domain_part;
        }
        /* copy scheme + truncated domain into tmp */
        if (scheme_len + use_domain_len + 1 > sizeof(tmp)) {
            /* Shouldn't happen due to calculations, but guard anyway */
            return fallback_empty;
        }
        memcpy(tmp, scheme, scheme_len);
        memcpy(tmp + scheme_len, domain_buf, use_domain_len);
        tmp[scheme_len + use_domain_len] = '\0';
    }

    /* Now allocate and return a heap copy */
    size_t out_len = strlen(tmp);
    char *out = (char*)malloc(out_len + 1);
    if (out == NULL) {
        /* malloc failed: return static empty string to be graceful */
        return fallback_empty;
    }
    memcpy(out, tmp, out_len + 1);
    return out;
}


// BEGIN ENTRYPOINT
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
