function get_welcome_message(username) {
  // Check if the username is empty or null
  if (!username || username.trim() === '') {
    return 'Welcome, Guest!';
  }

  // Generate the welcome message using the username
  return 'Welcome, ' + username + '!';
}


// BEGIN ENTRYPOINT
if (require.main === module) {
    const args = process.argv.slice(2);
    const username = args[0] || ''; // Default to an empty string if no username is provided
    console.log(get_welcome_message(username));
}
