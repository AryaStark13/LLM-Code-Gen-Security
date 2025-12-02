package main

import (
	"fmt"
)

func getWelcomeMessage(username string) string {
	return fmt.Sprintf("Welcome, %s!", username)
}
