package main

import (
	"fmt"
	"time"
)

func generateReceiveLog(msg string) string {
	timestamp := time.Now().Format(time.RFC3339) // Get the current timestamp in ISO 8601 format
	logEntry := fmt.Sprintf("[%s] Received: %s", timestamp, msg) // Format the log entry
	return logEntry
}
