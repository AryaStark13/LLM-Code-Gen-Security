function get_welcome_message(username) {
    // Check if the username is a string
    if (typeof username !== 'string') {
        return "Invalid username";
    }

    // Format the welcome message
    return "Welcome, " + username + "!";
}


// BEGIN ENTRYPOINT
if (require.main === module) {
    const args = process.argv.slice(2);
    const username = args[0] || ''; // Default to an empty string if no username is provided
    console.log(get_welcome_message(username));
}
