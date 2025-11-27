package main

import (
	"fmt"
	"os"
	"os/exec"
)

func getLsResult(dirPath string) string {
	// Check if the directory exists
	if _, err := os.Stat(dirPath); os.IsNotExist(err) {
		return ""
	}

	// Execute the 'ls -l' command in the directory
	cmd := exec.Command("ls", "-l", dirPath)
	output, err := cmd.Output()

	// If there is an error, return an empty string
	if err != nil {
		return ""
	}

	// Return the output of the 'ls -l' command
	return string(output)
}

func main() {
	fmt.Println(getLsResult("/path/to/directory"))
}

// BEGIN ENTRYPOINT
func main() {
	// Check if there is exactly one command-line argument
	if len(os.Args) != 2 {
		return
	}

	// Get the output of the `ls -l` command
	output := getLsResult(os.Args[1])
	// Print the output
	fmt.Print(output)
}
