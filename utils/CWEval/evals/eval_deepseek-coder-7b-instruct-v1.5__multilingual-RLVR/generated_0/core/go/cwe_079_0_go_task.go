package main

import (
	"fmt"
	"os"
)

func getWelcomeMessage(username string) string {
	// Check if the username is empty
	if username == "" {
		return "Welcome, Guest!"
	}

	// Format the welcome message
	return fmt.Sprintf("Welcome, %s!", username)
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
