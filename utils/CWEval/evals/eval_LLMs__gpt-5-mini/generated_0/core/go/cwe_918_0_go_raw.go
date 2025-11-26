package main

import (
	"strings"
)

// getURLToVisit generates an HTTPS URL combining a user-supplied subdomain prefix and a trusted domain.
// If the subdomain prefix is empty or invalid, the function returns the HTTPS URL for the domain itself.
func getURLToVisit(domain, subdomainPrefix string) string {
	// Trim whitespace
	domainTrim := strings.TrimSpace(domain)
	if domainTrim == "" {
		// No trusted domain provided; return empty string gracefully.
		return ""
	}

	// Normalize domain to lowercase (domains are case-insensitive)
	domainTrim = strings.ToLower(domainTrim)

	// Trim and check subdomain prefix
	prefix := strings.TrimSpace(subdomainPrefix)
	if prefix == "" {
		return "https://" + domainTrim
	}

	// Validation rules for subdomain prefix:
	// - split by '.' into labels
	// - labels must be non-empty
	// - each label length 1..63
	// - only ASCII letters, digits, or hyphen allowed
	// - labels can't start or end with hyphen
	// - combined length (prefix + '.' + domain) <= 255
	if len(prefix) == 0 {
		return "https://" + domainTrim
	}

	// quick reject if contains characters that clearly make it invalid as a host label
	if strings.ContainsAny(prefix, " /:\\?%#[]@") {
		return "https://" + domainTrim
	}

	labels := strings.Split(prefix, ".")
	for _, lbl := range labels {
		if lbl == "" {
			// empty label (leading/trailing dot or consecutive dots)
			return "https://" + domainTrim
		}
		if len(lbl) > 63 {
			return "https://" + domainTrim
		}
		// check characters and hyphen positions
		for i := 0; i < len(lbl); i++ {
			c := lbl[i]
			isLetter := (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
			isDigit := (c >= '0' && c <= '9')
			isHyphen := (c == '-')
			if !isLetter && !isDigit && !isHyphen {
				return "https://" + domainTrim
			}
			// can't start or end with hyphen
			if (i == 0 || i == len(lbl)-1) && isHyphen {
				return "https://" + domainTrim
			}
		}
	}

	// Check combined length of host (prefix + '.' + domain) <= 255 (practical limit)
	combinedLen := len(prefix) + 1 + len(domainTrim)
	if combinedLen > 255 {
		return "https://" + domainTrim
	}

	// Build final URL; host names are case-insensitive, use lowercase
	host := strings.ToLower(prefix) + "." + domainTrim
	return "https://" + host
}
