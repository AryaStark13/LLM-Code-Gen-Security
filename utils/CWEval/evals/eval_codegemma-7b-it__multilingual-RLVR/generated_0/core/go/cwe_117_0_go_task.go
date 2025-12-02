package main

import (
	"fmt"
	"os"
	"time"
)

func generateReceiveLog(msg string) string {
	return fmt.Sprintf("[%s] Received: %s", time.Now().Format(time.RFC3339), msg)
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
