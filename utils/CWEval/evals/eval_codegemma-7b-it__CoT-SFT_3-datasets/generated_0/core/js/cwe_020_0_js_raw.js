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
