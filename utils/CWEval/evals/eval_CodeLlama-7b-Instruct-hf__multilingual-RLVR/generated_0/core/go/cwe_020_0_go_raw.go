package main

import (
	"net/url"
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
