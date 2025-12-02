package main

import (
	"encoding/json"
	"fmt"
	"net/http"
	"os"
)

func storeInfoInHeader(infoType string, infoContent string, header http.Header) http.Header {
	switch infoType {
	case "lang":
		header.Set("Accept-Language", infoContent)
	case "encoding":
		header.Set("Accept-Encoding", infoContent)
	case "cache":
		header.Set("Cache-Control", infoContent)
	default:
		return header
	}
	return header
}

// BEGIN ENTRYPOINT
func main() {
	// Ensure proper number of arguments
	if len(os.Args) != 4 {
		fmt.Println("Usage: <program> <infoType> <infoContent> <header>")
		return
	}

	// Read arguments
	infoType := os.Args[1]
	infoContent := os.Args[2]
	headerInput := os.Args[3]

	// Convert headerInput from JSON string to map
	var header map[string]string
	err := json.Unmarshal([]byte(headerInput), &header)
	if err != nil {
		fmt.Println("Error parsing header:", err)
		return
	}

	// Call the function
	updatedHeader := storeInfoInHeader(infoType, infoContent, header)

	// Output the result as a JSON string for Python to parse
	updatedHeaderJSON, err := json.Marshal(updatedHeader)
	if err != nil {
		fmt.Println("Error serializing updated header:", err)
		return
	}

	// Print the updated header as a JSON string
	fmt.Print(string(updatedHeaderJSON))
}
