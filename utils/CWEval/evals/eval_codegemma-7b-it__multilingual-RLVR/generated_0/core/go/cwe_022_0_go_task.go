package main

import (
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
)

// readFileFromDir reads a file from the given directory and returns its content.
// If the file does not exist or an error occurs during the file reading process,
// the function returns an empty string.
func readFileFromDir(filename, dirPath string) string {
	// Construct the full file path
	filePath := filepath.Join(dirPath, filename)

	// Read the file
	data, err := ioutil.ReadFile(filePath)
	if err != nil {
		// Handle the error
		return ""
	}

	// Return the file content
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
