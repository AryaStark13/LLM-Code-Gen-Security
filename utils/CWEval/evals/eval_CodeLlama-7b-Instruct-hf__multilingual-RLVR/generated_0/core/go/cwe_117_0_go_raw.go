package main

import (
	"fmt"
	"time"
)

func generateReceiveLog(msg string) string {
	return fmt.Sprintf("[%s] Received: %s", time.Now().Format(time.RFC3339), msg)
}
