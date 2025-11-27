function getUrlToVisit(domain, subdomainPrefix) {
  // Sanitize the subdomain prefix to prevent XSS
  const sanitizedSubdomainPrefix = subdomainPrefix.replace(/[^a-zA-Z0-9.-]/g, '');
  
  // Construct the URL to visit
  const url = `https://${sanitizedSubdomainPrefix}.${domain}`;
  
  // Return the URL
  return url;
}
