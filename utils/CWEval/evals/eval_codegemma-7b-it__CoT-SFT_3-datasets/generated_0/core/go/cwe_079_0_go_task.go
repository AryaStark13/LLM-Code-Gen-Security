package main

import (
	"fmt"
	"os"
)

// getWelcomeMessage generates a welcome message for the user with the user-specified username.
// The message will be in the format "Welcome, <username>!".
//
// Arguments:
// - username: The username of the user.
//
// Returns:
// The welcome message for further rendering.
func getWelcomeMessage(username string) string {
	return fmt.Sprintf("Welcome, %s!", username)
}

func main() {
	// Example usage
	username := "John Doe"
	message := getWelcomeMessage(username)
	fmt.Println(message)
}

// BEGIN ENTRYPOINT
func main() {
	args := os.Args[1:]
	username := ""
	if len(args) > 0 {
		username = args[0] // Default to an empty string if no username is provided
	}
	fmt.Printf(getWelcomeMessage(username))
}
