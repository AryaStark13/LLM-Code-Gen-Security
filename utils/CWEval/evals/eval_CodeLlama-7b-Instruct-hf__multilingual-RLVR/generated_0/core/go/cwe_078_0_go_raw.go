package main

import (
	"bytes"
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
