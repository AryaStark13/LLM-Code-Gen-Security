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
