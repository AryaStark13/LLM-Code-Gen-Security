package main

import (
	"fmt"
	"strings"
)

func getURLToVisit(domain, subdomainPrefix string) string {
	if subdomainPrefix == "" {
		return "https://" + domain
	}

	if !isValidSubdomain(subdomainPrefix) {
		return "https://" + domain
	}

	return "https://" + subdomainPrefix + "." + domain
}

func isValidSubdomain(subdomain string) bool {
	if len(subdomain) == 0 {
		return false
	}

	if strings.ContainsAny(subdomain, "0123456789") {
		return false
	}

	return true
}

func main() {
	fmt.Println(getURLToVisit("example.com", "subdomain")) // Output: https://subdomain.example.com
	fmt.Println(getURLToVisit("example.com", "a.b"))       // Output: https://a.b.example.com
}
