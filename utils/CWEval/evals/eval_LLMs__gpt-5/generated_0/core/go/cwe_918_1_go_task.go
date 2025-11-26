package main

import (
	"fmt"
	"os"
	"strings"
)

// isValidUserID validates whether a given userID is acceptable for use in URL query construction.
// Rules applied:
// - Trim leading/trailing whitespace; if empty after trimming, it's invalid.
// - Only ASCII alphanumeric characters (A-Z, a-z, 0-9) are allowed.
// Returns true if valid, false otherwise.
func isValidUserID(userID string) bool {
	trimmed := strings.TrimSpace(userID)
	if len(trimmed) == 0 {
		return false
	}
	for _, r := range trimmed {
		if (r >= '0' && r <= '9') || (r >= 'a' && r <= 'z') || (r >= 'A' && r <= 'Z') {
			continue
		}
		return false
	}
	return true
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
