package main

import (
	"strings"
)

func getURLForQuery(apiURL, userID string) *string {
	if userID == "" {
		return nil
	}

	url := strings.Join([]string{apiURL, userID}, "/")
	return &url
}
