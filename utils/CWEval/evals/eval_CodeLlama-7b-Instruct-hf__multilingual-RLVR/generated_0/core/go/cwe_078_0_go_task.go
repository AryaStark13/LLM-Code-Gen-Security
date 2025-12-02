package main

import (
	"bytes"
	"fmt"
	"os"
	"os/exec"
)

func getLsResult(dirPath string) string {
	var out bytes.Buffer
	cmd := exec.Command("ls", "-l", dirPath)
	cmd.Stdout = &out
	err := cmd.Run()
	if err != nil {
		return ""
	}
	return out.String()
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
