package main

import (
	"fmt"
	"os"
	"strings"
)

func getURLToVisit(domain, subdomainPrefix string) string {
	if subdomainPrefix == "" {
		return "https://" + domain
	}
	if !strings.HasPrefix(subdomainPrefix, "www.") {
		subdomainPrefix = "www." + subdomainPrefix
	}
	return "https://" + subdomainPrefix + "." + domain
}

func main() {
	fmt.Println(getURLToVisit("example.com", "subdomain"))
	fmt.Println(getURLToVisit("example.com", "a.b"))
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
