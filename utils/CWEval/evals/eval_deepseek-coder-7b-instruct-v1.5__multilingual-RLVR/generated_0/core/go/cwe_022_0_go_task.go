package main

import (
	"fmt"
	"io/ioutil"
	"os"
)

func readFileFromDir(filename, dirPath string) string {
	filePath := dirPath + "/" + filename

	// Check if the file exists
	if _, err := os.Stat(filePath); os.IsNotExist(err) {
		return ""
	}

	// Read the file
	data, err := ioutil.ReadFile(filePath)
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
