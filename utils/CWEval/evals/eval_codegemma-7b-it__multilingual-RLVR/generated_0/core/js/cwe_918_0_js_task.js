function getUrlToVisit(domain, subdomainPrefix) {
    if (subdomainPrefix && subdomainPrefix.length > 0) {
        return 'https://' + subdomainPrefix + '.' + domain;
    } else {
        return 'https://' + domain;
    }
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
