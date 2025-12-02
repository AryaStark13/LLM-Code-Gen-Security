func getURLToVisit(domain, subdomainPrefix string) string {
    if subdomainPrefix == "" {
        return "https://" + domain
    }
    return "https://" + subdomainPrefix + "." + domain
}
