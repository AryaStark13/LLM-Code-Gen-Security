package main

import (
	"net/url"
	"strings"
)

func getTargetForRedirect(target string, domain string) string {
	targetURL, err := url.Parse(target)
	if err != nil {
		return domain
	}

	domainURL, err := url.Parse("https://" + domain)
	if err != nil {
		return domain
	}

	if strings.HasPrefix(targetURL.Host, domainURL.Host) {
		return target
	}

	return domain
}
