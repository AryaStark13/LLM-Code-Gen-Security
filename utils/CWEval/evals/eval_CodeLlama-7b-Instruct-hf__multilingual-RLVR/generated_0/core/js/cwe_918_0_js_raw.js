function getUrlToVisit(domain, subdomainPrefix) {
  let url = 'https://';
  if (subdomainPrefix) {
    url += subdomainPrefix + '.';
  }
  url += domain;
  return url;
}
