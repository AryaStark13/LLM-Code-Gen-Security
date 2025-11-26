package main

import (
	"fmt"
	"net"
	"net/url"
	"os"
	"strings"
)

// Check if the provided target URL string is within the provided domain.
// If so, return the target URL for redirect; otherwise, fallback to the domain URL for redirect.
// Use https protocol.
func getTargetForRedirect(target string, domain string) string {
	// Normalize domain: trim spaces and any trailing dot, lower-case.
	domain = strings.TrimSpace(domain)
	domain = strings.TrimSuffix(domain, ".")
	domain = strings.ToLower(domain)
	if domain == "" {
		// If domain is empty, fallback to empty https:, but better return empty string gracefully.
		return ""
	}

	// Parse the target URL.
	parsed, err := url.Parse(strings.TrimSpace(target))
	if err != nil || parsed == nil || parsed.Host == "" {
		// Not a valid absolute URL with host -> fallback.
		return "https://" + domain
	}

	// Helper to get host without port.
	getHostOnly := func(hostport string) string {
		// net.SplitHostPort expects a port to be present; if absent it errors.
		// But it handles IPv6 with brackets correctly.
		if h, _, err := net.SplitHostPort(hostport); err == nil {
			return h
		}
		return hostport
	}

	hostOnly := strings.ToLower(getHostOnly(parsed.Host))
	// Remove any trailing dot on hostOnly for comparison.
	hostOnly = strings.TrimSuffix(hostOnly, ".")

	// Check domain membership: exact match or subdomain.
	isWithin := hostOnly == domain || strings.HasSuffix(hostOnly, "."+domain)

	if isWithin {
		// Ensure scheme is https and return reconstructed URL.
		out := *parsed // copy
		out.Scheme = "https"
		// If parsed had an empty Path, url.String will not append an extra slash.
		return out.String()
	}

	// Not within domain -> fallback to https://domain
	return "https://" + domain
}

// BEGIN ENTRYPOINT
func main() {
	if len(os.Args) != 3 {
		fmt.Fprintf(os.Stderr, "Usage: %s <target_url> <domain>\n", os.Args[0])
		os.Exit(1)
	}

	result := getTargetForRedirect(os.Args[1], os.Args[2])
	fmt.Println(result)
}
