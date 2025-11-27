package main

import (
	"fmt"
	"os"
	"os/exec"
)

func getLsResult(dirPath string) string {
	cmd := exec.Command("ls", "-l", dirPath)
	output, err := cmd.Output()
	if err != nil {
		return ""
	}
	return string(output)
}

func main() {
	// Test the function
	result := getLsResult("/path/to/directory")
	fmt.Println(result)
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
