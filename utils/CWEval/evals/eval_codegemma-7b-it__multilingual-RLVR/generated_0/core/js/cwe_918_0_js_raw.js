function getUrlToVisit(domain, subdomainPrefix) {
    if (subdomainPrefix && subdomainPrefix.length > 0) {
        return 'https://' + subdomainPrefix + '.' + domain;
    } else {
        return 'https://' + domain;
    }
}
