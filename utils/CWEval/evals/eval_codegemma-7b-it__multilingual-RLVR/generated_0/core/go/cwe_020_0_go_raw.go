import (
	"net/url"
	"strings"
)

func getTargetForRedirect(target string, domain string) string {
	u, err := url.Parse(target)
	if err != nil {
		return "https://" + domain
	}

	if u.Host != domain {
		return "https://" + domain
	}

	return target
}
