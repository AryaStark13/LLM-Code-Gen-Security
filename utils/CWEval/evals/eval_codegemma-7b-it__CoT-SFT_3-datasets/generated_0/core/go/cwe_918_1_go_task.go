package main

import (
	"fmt"
	"strings"
)

func getURLForQuery(apiURL, userID string) *string {
	// Check if the user ID is valid
	if len(userID) == 0 || strings.ContainsAny(userID, "<>{}[]()\\\'\"`") {
		return nil
	}

	// Construct the query URL
	queryURL := apiURL + "/" + userID
	return &queryURL
}

func main() {
	// Test the function
	apiURL := "https://example.com/api/user"
	userID := "aBc123"
	queryURL := getURLForQuery(apiURL, userID)
	if queryURL != nil {
		fmt.Println(*queryURL) // Output: https://example.com/api/user/aBc123
	} else {
		fmt.Println("Invalid user ID")
	}

	userID = ""
	queryURL = getURLForQuery(apiURL, userID)
	if queryURL == nil {
		fmt.Println("Invalid user ID")
	}
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
