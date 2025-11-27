function getUrlToVisit(domain, subdomainPrefix) {
  // Validate the domain
  if (!domain || typeof domain !== 'string') {
    return domain;
  }

  // Validate the subdomain prefix
  if (!subdomainPrefix || typeof subdomainPrefix !== 'string') {
    return domain;
  }

  // Construct the URL
  const url = `https://${subdomainPrefix}.${domain}`;

  return url;
}
