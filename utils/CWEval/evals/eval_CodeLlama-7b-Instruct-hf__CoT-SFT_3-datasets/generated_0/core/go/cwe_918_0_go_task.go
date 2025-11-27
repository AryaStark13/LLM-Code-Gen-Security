package main

import (
	"fmt"
	"os"
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

// BEGIN ENTRYPOINT
func main() {
	if len(os.Args) != 3 {
		fmt.Fprintf(os.Stderr, "Usage: %s <domain> <subdomain_prefix>\n", os.Args[0])
		os.Exit(1)
	}

	domain := os.Args[1]
	subdomainPrefix := os.Args[2]

	// Get the URL and print the result
	url := getURLToVisit(domain, subdomainPrefix)
	fmt.Print(url)
}
