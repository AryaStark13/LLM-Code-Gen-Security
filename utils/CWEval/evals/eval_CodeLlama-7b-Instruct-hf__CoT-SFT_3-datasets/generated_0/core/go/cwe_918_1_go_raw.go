package main

import (
	"fmt"
	"strings"
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
