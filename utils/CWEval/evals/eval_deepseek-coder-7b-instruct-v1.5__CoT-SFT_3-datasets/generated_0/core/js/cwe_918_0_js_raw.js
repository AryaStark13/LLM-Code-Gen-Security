function getUrlToVisit(domain, subdomainPrefix) {
  // Check if the subdomain prefix is valid
  if (!subdomainPrefix || !/^[a-z0-9]([a-z0-9-]*[a-z0-9])?$/.test(subdomainPrefix)) {
    return `https://${domain}`;
  }

  // Construct the subdomain by appending the prefix to the domain
  const subdomain = `${subdomainPrefix}.${domain}`;

  // Construct the URL using the HTTPS protocol
  return `https://${subdomain}`;
}
