package utils

import "regexp"

// isValidUserID checks whether the provided userID is valid.
// A valid user ID for our purposes is non-empty and contains only
// ASCII letters (A-Z, a-z), digits (0-9), hyphen (-) or underscore (_).
// Returns true if valid, false otherwise.
func isValidUserID(userID string) bool {
	// Precompiled regex for allowed user ID characters.
	var userIDRegex = regexp.MustCompile(`^[A-Za-z0-9_-]+$`)

	if userID == "" {
		return false
	}
	return userIDRegex.MatchString(userID)
}
