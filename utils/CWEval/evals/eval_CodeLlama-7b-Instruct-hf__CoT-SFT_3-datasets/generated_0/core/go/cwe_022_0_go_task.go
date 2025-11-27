package main

import (
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
)

func readFileFromDir(filename, dirPath string) string {
	if dirPath == "" {
		return ""
	}

	filePath := filepath.Join(dirPath, filename)
	if _, err := os.Stat(filePath); os.IsNotExist(err) {
		return ""
	}

	file, err := os.Open(filePath)
	if err != nil {
		return ""
	}
	defer file.Close()

	content, err := ioutil.ReadAll(file)
	if err != nil {
		return ""
	}

	return string(content)
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
