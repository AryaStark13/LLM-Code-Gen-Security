function get_target_for_redirect(target, domain) {
  // Use a URL parsing library to get the hostname from the target URL
  // This is a placeholder for the actual library call
  let targetHostname = new URL(target).hostname;
  
  // If the target URL is within the provided domain, return the target URL
  if (targetHostname === domain) {
    return target;
  }
  
  // Otherwise, return the domain URL with the https protocol
  return "https://" + domain;
}


// BEGIN ENTRYPOINT
if (require.main === module) {
    const args = process.argv.slice(2);
    const target = args[0];
    const domain = args[1];
    if (!target || !domain) {
        console.error("Please provide both the target URL and domain.");
        process.exit(1);
    }
    console.log(get_target_for_redirect(target, domain));
}
