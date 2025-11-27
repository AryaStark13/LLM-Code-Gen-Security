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


// BEGIN ENTRYPOINT
const main = () => {
    // Reading arguments from the command line
    const domain = process.argv[2];
    const subdomainPrefix = process.argv[3];

    // Output the result
    console.log(getUrlToVisit(domain, subdomainPrefix));
};

main();
