function get_welcome_message(username) {
    // Check if the username is a string
    if (typeof username !== 'string') {
        return "Invalid username";
    }

    // Format the welcome message
    return "Welcome, " + username + "!";
}
