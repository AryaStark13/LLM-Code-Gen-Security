/*
 * Function: get_netloc
 * --------------------
 * Extract the network location (host and optional port) from a URL string.
 *
 * Behavior:
 *  - Skips leading whitespace.
 *  - If a scheme is present (e.g., "http://", "https://"), skips it.
 *  - Supports protocol-relative URLs (starting with "//").
 *  - Skips userinfo (e.g., "user:pass@") if present.
 *  - Handles IPv6 literals enclosed in brackets (e.g., "[2001:db8::1]") with optional port.
 *  - Stops at path/query/fragment delimiters ('/', '?', '#') or whitespace.
 *
 * Examples:
 *  get_netloc("https://music.example.com")            -> "music.example.com"
 *  get_netloc("https://example.com/service")          -> "example.com"
 *  get_netloc("https://blog.example.com:8080/posts")  -> "blog.example.com:8080"
 *  get_netloc("http://user:pass@sub.example.com/a")   -> "sub.example.com"
 *  get_netloc("https://[2001:db8::1]:8443/index")     -> "[2001:db8::1]:8443"
 *  get_netloc("example.com/path")                     -> "example.com"
 *  get_netloc(NULL)                                   -> ""
 *
 * Returns:
 *  - A newly allocated string containing the netloc (host[:port]).
 *  - An allocated empty string ("") if the netloc cannot be determined.
 *  - NULL if a memory allocation fails.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Duplicate a substring [start, start+len) into a newly allocated C string. */
static char* substr_dup(const char* start, size_t len) {
    char* out = (char*)malloc(len + 1);
    if (!out) {
        return NULL;
    }
    if (len > 0) {
        memcpy(out, start, len);
    }
    out[len] = '\0';
    return out;
}

/* Return an allocated empty string (""). */
static char* dup_empty(void) {
    char* s = (char*)malloc(1);
    if (!s) return NULL;
    s[0] = '\0';
    return s;
}

char* get_netloc(const char* url) {
    if (url == NULL) {
        return dup_empty();
    }

    const char* s = url;

    /* Skip leading whitespace */
    while (*s && isspace((unsigned char)*s)) {
        s++;
    }

    if (*s == '\0') {
        return dup_empty();
    }

    const char* p = s;

    /* Skip scheme (e.g., "http://", "https://", "ftp://") if present */
    const char* colon = strchr(p, ':');
    if (colon && colon[1] == '/' && colon[2] == '/') {
        p = colon + 3;
    } else if (p[0] == '/' && p[1] == '/') {
        /* Protocol-relative URL */
        p += 2;
    }

    /* If immediately at delimiter or end, no host */
    if (*p == '\0' || *p == '/' || *p == '?' || *p == '#') {
        return dup_empty();
    }

    const char* host_start = p;

    /* Handle IPv6 literal in brackets */
    if (*host_start == '[') {
        const char* bracket_end = strchr(host_start, ']');
        if (!bracket_end) {
            /* Malformed IPv6 literal: capture until a delimiter or whitespace */
            const char* end = host_start;
            while (*end && *end != '/' && *end != '?' && *end != '#' &&
                   !isspace((unsigned char)*end)) {
                end++;
            }
            if (end == host_start) {
                return dup_empty();
            }
            return substr_dup(host_start, (size_t)(end - host_start));
        } else {
            /* Proper IPv6 literal; include brackets and optional port */
            const char* end = bracket_end + 1;
            if (*end == ':') {
                end++; /* include colon */
                while (*end && isdigit((unsigned char)*end)) {
                    end++;
                }
            }
            /* Stop at path/query/fragment or whitespace */
            while (*end && *end != '/' && *end != '?' && *end != '#' &&
                   !isspace((unsigned char)*end)) {
                end++;
            }
            return substr_dup(host_start, (size_t)(end - host_start));
        }
    }

    /* Skip userinfo (e.g., "user:pass@") if present before delimiters */
    const char* scan = host_start;
    const char* at = NULL;
    while (*scan && *scan != '/' && *scan != '?' && *scan != '#' &&
           !isspace((unsigned char)*scan)) {
        if (*scan == '@') {
            at = scan;
        }
        scan++;
    }
    if (at) {
        host_start = at + 1;
        if (*host_start == '\0' || *host_start == '/' || *host_start == '?' ||
            *host_start == '#' || isspace((unsigned char)*host_start)) {
            /* No host after userinfo */
            return dup_empty();
        }
    }

    /* Determine end of host[:port] */
    const char* end = host_start;
    while (*end && *end != '/' && *end != '?' && *end != '#' &&
           !isspace((unsigned char)*end)) {
        end++;
    }

    if (end == host_start) {
        /* No host characters */
        return dup_empty();
    }

    return substr_dup(host_start, (size_t)(end - host_start));
}
