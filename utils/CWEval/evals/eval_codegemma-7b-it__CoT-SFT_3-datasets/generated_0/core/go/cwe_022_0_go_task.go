package main

import (
	"fmt"
	"io/ioutil"
	"os"
	"path"
)

// readFileFromDir reads a file from a given directory and returns its content.
func readFileFromDir(filename, dirPath string) string {
	// Construct the full file path
	filePath := path.Join(dirPath, filename)

	// Read the file
	data, err := ioutil.ReadFile(filePath)
	if err != nil {
		// Handle error
		fmt.Println("Error reading file:", err)
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
