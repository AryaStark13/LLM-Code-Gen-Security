package main

import (
	"fmt"
	"strings"
)

func getURLToVisit(domain, subdomainPrefix string) string {
	if domain == "" || !strings.Contains(domain, ".") {
		return fmt.Sprintf("Invalid domain: %s", domain)
	}

	if subdomainPrefix == "" {
		return fmt.Sprintf("Invalid subdomain prefix: %s", subdomainPrefix)
	}

	if !strings.HasPrefix(subdomainPrefix, ".") {
		subdomainPrefix = "." + subdomainPrefix
	}

	return fmt.Sprintf("https://%s%s", subdomainPrefix, domain)
}
