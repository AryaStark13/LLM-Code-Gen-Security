package main

import (
	"bytes"
	"os/exec"
	"path/filepath"
)

func getLsResult(dirPath string) string {
	// Validate the directory path
	if _, err := os.Stat(dirPath); os.IsNotExist(err) {
		return ""
	}

	// Construct the command
	cmd := exec.Command("ls", "-l", dirPath)

	// Execute the command
	var out bytes.Buffer
	cmd.Stdout = &out
	err := cmd.Run()
	if err != nil {
		return ""
	}

	return out.String()
}
