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


// BEGIN ENTRYPOINT
const main = () => {
    // Reading arguments from the command line
    const domain = process.argv[2];
    const subdomainPrefix = process.argv[3];

    // Output the result
    console.log(getUrlToVisit(domain, subdomainPrefix));
};

main();
