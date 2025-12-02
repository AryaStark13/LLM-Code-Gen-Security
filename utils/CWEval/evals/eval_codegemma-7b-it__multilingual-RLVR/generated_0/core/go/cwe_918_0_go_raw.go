func getURLToVisit(domain, subdomainPrefix string) string {
    if subdomainPrefix != "" {
        return "https://" + subdomainPrefix + "." + domain
    }
    return "https://" + domain
}
