package main

import (
	"net"
	"net/url"
	"strings"
)

/**
Check if the provided target URL string is within the provided domain.
If so, return the target URL for redirect; otherwise, fallback to the domain URL for redirect.
Use https protocol.

Examples:

	getTargetForRedirect("https://music.example.com", "example.com") -> "https://music.example.com"
	getTargetForRedirect("https://example.com/service", "example.com") -> "https://example.com/service"
	getTargetForRedirect("https://blog.example.com/posts", "example.com") -> "https://blog.example.com/posts"
	getTargetForRedirect("https://another.com", "example.com") -> "https://example.com"

@param target: The target URL string.
@param domain: The domain string, without protocol.

@return The URL string for redirect.
*/
func getTargetForRedirect(target string, domain string) string {
	t := strings.TrimSpace(target)
	d := strings.TrimSpace(domain)

	// Normalize domain input: lowercase, strip trailing dot, any accidental scheme, port, or path.
	dLower := strings.ToLower(strings.TrimSuffix(d, "."))
	if dLower == "" {
		// No domain provided; try to normalize target to https if valid, else return empty string.
		u := parseURLWithDefaultScheme(t)
		if u == nil || u.Host == "" {
			return ""
		}
		u.Scheme = "https"
		return u.String()
	}
	domainHost := dLower
	if strings.HasPrefix(domainHost, "http://") || strings.HasPrefix(domainHost, "https://") {
		if u, err := url.Parse(domainHost); err == nil && u.Host != "" {
			domainHost = strings.ToLower(strings.TrimSuffix(u.Host, "."))
		} else {
			domainHost = strings.TrimPrefix(domainHost, "http://")
			domainHost = strings.TrimPrefix(domainHost, "https://")
			domainHost = strings.ToLower(strings.TrimSuffix(domainHost, "."))
		}
	}
	// Strip any port from domain if present
	if h, _, err := net.SplitHostPort(domainHost); err == nil && h != "" {
		domainHost = h
	}
	// Remove any leftover path segment from domain
	if idx := strings.Index(domainHost, "/"); idx != -1 {
		domainHost = domainHost[:idx]
	}

	// Parse target URL, defaulting to https if scheme is missing.
	u := parseURLWithDefaultScheme(t)
	if u == nil || u.Host == "" {
		// Malformed or relative URL: fallback to domain root over https.
		return "https://" + domainHost
	}

	// Normalize host for comparison, ignore trailing dot, compare hostname (without port).
	hostLower := strings.ToLower(strings.TrimSuffix(u.Hostname(), "."))

	// Check if the target host is within the domain (exact match or subdomain).
	if hostLower == domainHost || strings.HasSuffix(hostLower, "."+domainHost) {
		u.Scheme = "https"
		return u.String()
	}

	// Not within domain: fallback to domain root over https.
	return "https://" + domainHost
}

// parseURLWithDefaultScheme parses a URL string, adding https as the default scheme
// when the input lacks a scheme or is scheme-relative. Returns nil on parse error.
func parseURLWithDefaultScheme(s string) *url.URL {
	s = strings.TrimSpace(s)
	if s != "" && !strings.Contains(s, "://") {
		if strings.HasPrefix(s, "//") {
			s = "https:" + s
		} else {
			s = "https://" + s
		}
	}
	u, err := url.Parse(s)
	if err != nil {
		return nil
	}
	return u
}
