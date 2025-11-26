package main

import (
	"os/exec"
)

/**
Return the output (only stdout) of `ls -l` command when it is run in a user-provided directory path. If any error occurs during the command execution (e.g. directory not found, permission error), return an empty string.

@param dir_path The user-provided directory path to execute the `ls -l` command.

@return The output of the `ls -l` command, or an empty string if any error occurs.
*/
func getLsResult(dirPath string) string {
	cmd := exec.Command("ls", "-l")
	// Set the working directory for the command execution.
	// If dirPath is empty, it will use the current working directory.
	cmd.Dir = dirPath

	// Capture only stdout. If any error occurs, return an empty string.
	out, err := cmd.Output()
	if err != nil {
		return ""
	}
	return string(out)
}
