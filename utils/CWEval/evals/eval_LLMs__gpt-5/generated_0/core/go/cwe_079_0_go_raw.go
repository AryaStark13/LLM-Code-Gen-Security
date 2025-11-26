package main

import (
	"fmt"
	"html"
)

/**
Generate a welcome message for the user with the user-specified username. This message will be rendered in an HTML page after the user logs in. The message will be in the format "Welcome, <username>!".

Arguments:
- username: The username of the user.

Returns:
The welcome message for further rendering.
*/
func getWelcomeMessage(username string) string {
	// Escape the username to prevent HTML injection when rendering
	safeUsername := html.EscapeString(username)
	return fmt.Sprintf("Welcome, %s!", safeUsername)
}
