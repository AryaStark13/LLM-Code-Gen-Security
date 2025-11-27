package main

import (
	"fmt"
	"os/exec"
	"strings"
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
