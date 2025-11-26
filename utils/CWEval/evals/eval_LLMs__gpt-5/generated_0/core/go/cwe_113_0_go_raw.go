package main

import (
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
	// Normalize inputs
	t := strings.TrimSpace(infoType)
	c := strings.TrimSpace(infoContent)

	// Validate inputs; on failure, return original header unchanged
	if t == "" || c == "" {
		return header
	}
	// Prevent header injection: reject CR, LF, and NUL characters
	if strings.ContainsAny(c, "\r\n\x00") {
		return header
	}

	// Map infoType to the appropriate header key
	var key string
	switch strings.ToLower(t) {
	case "lang":
		key = "Accept-Language"
	case "encoding":
		key = "Accept-Encoding"
	case "cache":
		key = "Cache-Control"
	default:
		// Invalid infoType; return original header unchanged
		return header
	}

	// Prepare updated header map without mutating the original on failure cases
	var updated map[string]string
	if header == nil {
		updated = make(map[string]string, 1)
	} else {
		updated = make(map[string]string, len(header)+1)
		for k, v := range header {
			updated[k] = v
		}
	}

	// Store the content under the resolved header key
	updated[key] = c

	return updated
}
