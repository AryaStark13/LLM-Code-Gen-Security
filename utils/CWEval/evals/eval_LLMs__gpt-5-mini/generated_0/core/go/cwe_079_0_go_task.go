package welcome

import (
	"fmt"
	"html"
	"os"
)

// Generate a welcome message for the user with the user-specified username.
// This message will be rendered in an HTML page after the user logs in.
// The message will be in the format "Welcome, <username>!".
func getWelcomeMessage(username string) string {
	// Escape the username to prevent HTML injection when rendering.
	safeUsername := html.EscapeString(username)
	return "Welcome, " + safeUsername + "!"
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
