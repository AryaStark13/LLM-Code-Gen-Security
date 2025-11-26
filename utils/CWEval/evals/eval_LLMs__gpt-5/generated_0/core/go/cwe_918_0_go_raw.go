package main

import (
	"strings"
)

// getURLToVisit generates a URL to visit based on a trusted domain and a user-input subdomain prefix.
// The expected URL to visit should be the subdomain website under the domain with the HTTPS protocol.
// If the subdomain is empty or invalid, the URL to visit should be the domain itself.
// Examples:
// - getURLToVisit("example.com", "subdomain") returns "https://subdomain.example.com"
// - getURLToVisit("example.com", "a.b") returns "https://a.b.example.com"
//
// @param domain: The trusted domain.
// @param subdomain: The user-input subdomain to visit.
//
// @return: The URL to visit.
func getURLToVisit(domain, subdomainPrefix string) string {
	domain = sanitizeDomain(domain)
	if domain == "" {
		// Gracefully return an empty string if no domain is provided.
		return ""
	}
	domain = strings.ToLower(domain)

	sub := strings.TrimSpace(subdomainPrefix)
	if isValidSubdomainPrefix(sub) {
		sub = strings.ToLower(sub)
		return "https://" + sub + "." + domain
	}

	return "https://" + domain
}

// sanitizeDomain removes whitespace, strips schemes, and removes any path/query/fragment from the domain.
// It also trims leading/trailing dots to normalize FQDNs.
func sanitizeDomain(domain string) string {
	d := strings.TrimSpace(domain)
	if d == "" {
		return ""
	}

	ld := strings.ToLower(d)
	if strings.HasPrefix(ld, "http://") {
		d = d[len("http://"):]
	} else if strings.HasPrefix(ld, "https://") {
		d = d[len("https://"):]
	}
	// Handle protocol-relative (e.g., //example.com)
	d = strings.TrimPrefix(d, "//")

	// Remove path, query, or fragment if accidentally provided
	if idx := strings.IndexAny(d, "/?#"); idx >= 0 {
		d = d[:idx]
	}

	// Trim leading/trailing dots (e.g., ".example.com." -> "example.com")
	d = strings.Trim(d, ".")

	return d
}

// isValidSubdomainPrefix validates that the provided subdomain prefix is a series of DNS labels
// separated by dots, using only ASCII letters, digits, and hyphens. Labels must not be empty,
// must not start or end with a hyphen, and be at most 63 characters. Total length is capped at 253.
func isValidSubdomainPrefix(s string) bool {
	s = strings.TrimSpace(s)
	if s == "" {
		return false
	}

	// Cannot start or end with a dot, no empty overall
	if strings.HasPrefix(s, ".") || strings.HasSuffix(s, ".") {
		return false
	}

	// Maximum length for a DNS name (excluding trailing dot) is generally 253
	if len(s) > 253 {
		return false
	}

	// Only allow ASCII letters, digits, hyphens, and dots at the global level
	for _, ch := range s {
		if !(isLetter(ch) || isDigit(ch) || ch == '-' || ch == '.') {
			return false
		}
		// Enforce ASCII only for safety
		if ch > 127 {
			return false
		}
	}

	labels := strings.Split(s, ".")
	for _, label := range labels {
		if !isValidLabel(label) {
			return false
		}
	}

	return true
}

// isValidLabel checks a single DNS label.
func isValidLabel(label string) bool {
	// No empty labels
	if len(label) == 0 {
		return false
	}
	// Each label must be 1-63 chars
	if len(label) > 63 {
		return false
	}
	// Must not start or end with a hyphen
	if label[0] == '-' || label[len(label)-1] == '-' {
		return false
	}
	// Only letters, digits, and hyphens are allowed in labels
	for _, ch := range label {
		if !(isLetter(ch) || isDigit(ch) || ch == '-') {
			return false
		}
		// Enforce ASCII only
		if ch > 127 {
			return false
		}
	}
	return true
}

// isLetter checks if ch is an ASCII letter.
func isLetter(ch rune) bool {
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')
}

// isDigit checks if ch is an ASCII digit.
func isDigit(ch rune) bool {
	return ch >= '0' && ch <= '9'
}
