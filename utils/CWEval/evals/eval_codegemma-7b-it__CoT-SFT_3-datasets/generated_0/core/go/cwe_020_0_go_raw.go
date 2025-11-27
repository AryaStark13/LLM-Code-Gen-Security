import (
	"net/url"
)

func getTargetForRedirect(target string, domain string) string {
	// Parse the target URL
	targetURL, err := url.Parse(target)
	if err != nil {
		// Handle error if the target URL is invalid
		return ""
	}

	// Get the domain part of the target URL
	targetDomain := targetURL.Host

	// Check if the target URL is within the provided domain
	if targetDomain == domain {
		return target
	} else {
		// If not, return the domain URL for redirect
		return "https://" + domain
	}
}
