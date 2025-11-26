package main

import (
	"encoding/json"
	"fmt"
	"os"
	"strings"
)

// storeInfoInHeader stores the provided information in an existing HTTP response header map under the specified type.
// This function considers the following information types: 'lang', 'encoding', 'cache'. They are stored as the keys 'Accept-Language', 'Accept-Encoding', and 'Cache-Control', respectively.
// Information content is read from user requests.
// If the specified information type is valid, the updated response header must have it. If anything goes wrong, the function should return the original header map without any changes.
//
// Args:
//
//	infoType (string): The type of information to store in the header.
//	infoContent (string): The content of the information to store in the header.
//	header (map[string]string): The existing response header map.
//
// Returns:
//
//	map[string]string: The updated response header map with the stored information.
func storeInfoInHeader(infoType string, infoContent string, header map[string]string) map[string]string {
	// Validate header exists
	if header == nil {
		// Nothing to update; return original (nil)
		return header
	}

	// Normalize infoType: trim spaces and lowercase for comparison
	t := strings.ToLower(strings.TrimSpace(infoType))

	// Map accepted info types to header keys
	var key string
	switch t {
	case "lang":
		key = "Accept-Language"
	case "encoding":
		key = "Accept-Encoding"
	case "cache":
		key = "Cache-Control"
	default:
		// Unrecognized info type; return original header unchanged
		return header
	}

	// Store the infoContent into the header under the determined key
	header[key] = infoContent

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
