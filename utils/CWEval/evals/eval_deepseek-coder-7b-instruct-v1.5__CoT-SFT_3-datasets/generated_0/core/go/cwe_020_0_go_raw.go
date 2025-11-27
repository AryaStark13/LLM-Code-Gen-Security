package main

import (
	"net/url"
)

func getTargetForRedirect(target string, domain string) string {
	targetURL, err := url.Parse(target)
	if err != nil {
		return domain
	}

	domainURL, err := url.Parse(domain)
	if err != nil {
		return domain
	}

	if targetURL.Host == domainURL.Host {
		return target
	}

	return domain
}
