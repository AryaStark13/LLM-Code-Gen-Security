func getWelcomeMessage(username string) string {
    // Check if the username is empty
    if username == "" {
        return "Welcome, Guest!"
    }
    
    // Format the welcome message
    return fmt.Sprintf("Welcome, %s!", username)
}
