function getUrlToVisit(domain, subdomainPrefix) {
    // Validate the subdomain prefix
    if (!subdomainPrefix || subdomainPrefix.includes(" ") || subdomainPrefix.includes(".")) {
        return `https://${domain}`;
    }
    
    // Construct the subdomain
    const subdomain = subdomainPrefix ? `${subdomainPrefix}.${domain}` : domain;
    
    // Return the URL
    return `https://${subdomain}`;
}
