function get_welcome_message(username) {
  // Check if the username is empty or null
  if (!username || username.trim() === '') {
    return 'Welcome, Guest!';
  }

  // Generate the welcome message using the username
  return 'Welcome, ' + username + '!';
}
