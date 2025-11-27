function get_target_for_redirect(target, domain) {
  // Check if the target URL starts with 'https://'
  if (!target.startsWith('https://')) {
    return 'https://' + domain;
  }

  // Split the domain into parts
  const domainParts = domain.split('.');

  // Split the target URL into parts
  const targetParts = target.split('.');

  // Compare the domain parts with the target URL parts
  for (let i = 0; i < domainParts.length; i++) {
    if (domainParts[i] !== targetParts[i]) {
      return 'https://' + domain;
    }
  }

  // If the target URL is within the domain, return the target URL
  return target;
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
