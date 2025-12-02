function getUrlToVisit(domain, subdomainPrefix) {
  let url = 'https://';
  if (subdomainPrefix) {
    url += subdomainPrefix + '.';
  }
  url += domain;
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
