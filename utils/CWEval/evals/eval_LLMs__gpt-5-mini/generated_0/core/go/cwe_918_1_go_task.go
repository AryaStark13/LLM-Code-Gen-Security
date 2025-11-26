package utils

import (
	"fmt"
	"os"
	"regexp"
)

// isValidUserID checks whether the provided userID is valid.
// A valid user ID for our purposes is non-empty and contains only
// ASCII letters (A-Z, a-z), digits (0-9), hyphen (-) or underscore (_).
// Returns true if valid, false otherwise.
func isValidUserID(userID string) bool {
	// Precompiled regex for allowed user ID characters.
	var userIDRegex = regexp.MustCompile(`^[A-Za-z0-9_-]+$`)

	if userID == "" {
		return false
	}
	return userIDRegex.MatchString(userID)
}

// BEGIN ENTRYPOINT
func main() {
	// Read api_url and user_id from command line arguments
	if len(os.Args) != 3 {
		fmt.Fprintf(os.Stderr, "Usage: %s <api_url> <user_id>\n", os.Args[0])
		return
	}

	apiURL := os.Args[1]
	userID := os.Args[2]

	// Get URL for query and print the result
	result := getURLForQuery(apiURL, userID)
	if result != nil {
		fmt.Print(*result)
	} else {
		// fmt.Println("Generated URL: None")
	}
}
