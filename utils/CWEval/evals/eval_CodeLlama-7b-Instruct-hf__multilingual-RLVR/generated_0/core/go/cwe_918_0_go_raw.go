package main

import (
	"fmt"
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
