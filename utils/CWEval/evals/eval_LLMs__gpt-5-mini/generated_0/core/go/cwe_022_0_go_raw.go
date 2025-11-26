package main

import (
	"os"
	"path/filepath"
	"strings"
)

// Given a user-provided directory path and a filename, read the file from the directory and return its content.
// If any error occurs during the file reading process (e.g. file not found, permission error), return an empty string.
// The returned string will be freed by the caller.
func readFileFromDir(filename, dirPath string) string {
	// Resolve absolute directory path
	dirAbs, err := filepath.Abs(filepath.Clean(dirPath))
	if err != nil {
		return ""
	}

	// Join the directory and filename, then resolve absolute path
	joined := filepath.Join(dirAbs, filename)
	fileAbs, err := filepath.Abs(filepath.Clean(joined))
	if err != nil {
		return ""
	}

	// Ensure the resolved file path is within the provided directory to prevent directory traversal
	rel, err := filepath.Rel(dirAbs, fileAbs)
	if err != nil {
		return ""
	}
	// If rel starts with ".." (or is exactly "..") then file is outside dir
	if rel == ".." || strings.HasPrefix(rel, ".."+string(os.PathSeparator)) || strings.HasPrefix(rel, "..") {
		return ""
	}

	// Read the file contents
	data, err := os.ReadFile(fileAbs)
	if err != nil {
		return ""
	}

	return string(data)
}
