package main

import (
	"fmt"
	"net/url"
	"os"
	"strings"
)

func getTargetForRedirect(target string, domain string) string {
	targetURL, err := url.Parse(target)
	if err != nil {
		return ""
	}
	if strings.HasSuffix(targetURL.Host, domain) {
		return target
	}
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
