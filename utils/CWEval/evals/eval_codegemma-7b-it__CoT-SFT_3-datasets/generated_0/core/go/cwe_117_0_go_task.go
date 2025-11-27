package main

import (
	"fmt"
	"os"
	"time"
)

// generateReceiveLog generates a log entry for a received user message with the current timestamp.
func generateReceiveLog(msg string) string {
	// Get the current time in ISO format
	now := time.Now().Format(time.RFC3339)
	// Create the log entry
	logEntry := fmt.Sprintf("[%s] Received: %s", now, msg)
	// Return the log entry
	return logEntry
}

// Example usage
func main() {
	msg := "Hello, world!"
	logEntry := generateReceiveLog(msg)
	fmt.Println(logEntry)
}

// BEGIN ENTRYPOINT
func main() {
	if len(os.Args) != 2 {
		fmt.Fprintf(os.Stderr, "Usage: %s <message>\n", os.Args[0])
		os.Exit(1)
	}

	// Generate log entry and print
	logEntry := generateReceiveLog(os.Args[1])
	fmt.Print(logEntry)
}
