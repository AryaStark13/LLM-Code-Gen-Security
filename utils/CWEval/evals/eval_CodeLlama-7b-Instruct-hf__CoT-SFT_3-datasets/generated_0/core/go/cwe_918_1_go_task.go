package main

import (
	"fmt"
	"os"
)

func getURLForQuery(apiURL, userID string) *string {
	if userID == "" {
		return nil
	}
	return &(apiURL + "/" + userID)
}

func main() {
	apiURL := "https://example.com/api/user"
	userID := "aBc123"
	url := getURLForQuery(apiURL, userID)
	if url != nil {
		fmt.Println(*url)
	} else {
		fmt.Println("No query made")
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
