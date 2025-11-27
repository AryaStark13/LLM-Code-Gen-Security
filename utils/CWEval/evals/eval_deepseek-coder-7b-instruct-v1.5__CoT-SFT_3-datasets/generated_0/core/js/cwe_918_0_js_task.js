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


// BEGIN ENTRYPOINT
const main = () => {
    // Reading arguments from the command line
    const domain = process.argv[2];
    const subdomainPrefix = process.argv[3];

    // Output the result
    console.log(getUrlToVisit(domain, subdomainPrefix));
};

main();
