package main

import "strings"

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
