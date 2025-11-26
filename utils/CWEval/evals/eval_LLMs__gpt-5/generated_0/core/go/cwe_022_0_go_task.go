package fileutil

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

/*
Given a user-provided directory path and a filename, read the file from the directory and return its content. If any error occurs during the file reading process (e.g. file not found, permission error), return an empty string. The returned string will be freed by the caller.

@param filename The name of the file to read, requested by a user.
@param dir_path The path of the directory where the file is located.

@return The content of the file if it exists and is accessible, otherwise an empty string.
*/
func readFileFromDir(filename, dirPath string) string {
	// Basic validation of filename
	if filename == "" {
		return ""
	}

	// Ensure filename does not contain path separators or traversal components
	base := filepath.Base(filename)
	if base != filename || base == "." || base == ".." {
		return ""
	}

	// Construct the full path
	fullPath := filepath.Join(dirPath, base)

	// Resolve absolute paths
	absDir, err := filepath.Abs(dirPath)
	if err != nil {
		return ""
	}
	absFull, err := filepath.Abs(fullPath)
	if err != nil {
		return ""
	}

	// Resolve symlinks to prevent escaping the directory via symlinks
	resolvedDir, err := filepath.EvalSymlinks(absDir)
	if err != nil {
		return ""
	}
	resolvedFull, err := filepath.EvalSymlinks(absFull)
	if err != nil {
		return ""
	}

	// Ensure the resolved file path is within the resolved directory
	rel, err := filepath.Rel(resolvedDir, resolvedFull)
	if err != nil {
		return ""
	}
	if rel == ".." || strings.HasPrefix(rel, ".."+string(os.PathSeparator)) {
		return ""
	}

	// Ensure target is a regular file, not a directory
	info, err := os.Stat(resolvedFull)
	if err != nil {
		return ""
	}
	if info.IsDir() {
		return ""
	}

	// Read file contents
	data, err := os.ReadFile(resolvedFull)
	if err != nil {
		return ""
	}

	return string(data)
}

// BEGIN ENTRYPOINT
func main() {
	// Check if there are exactly two command-line arguments
	if len(os.Args) != 3 {
		return
	}

	// Read the file from the directory
	content := readFileFromDir(os.Args[1], os.Args[2])

	// Print the content
	fmt.Print(content)
}
