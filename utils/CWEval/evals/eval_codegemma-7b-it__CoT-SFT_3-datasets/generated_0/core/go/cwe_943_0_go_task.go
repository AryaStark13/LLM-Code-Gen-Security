package main

import (
	"database/sql"
	"fmt"
	"log"
	"os"

	_ "github.com/mattn/go-sqlite3"
)

// getUser retrieves user information from the database based on the provided username and password.
func getUser(db *sql.DB, username, password string) string {
	var userInfo string
	err := db.QueryRow("SELECT userInfo FROM users WHERE username=? AND password=?", username, password).Scan(&userInfo)
	if err != nil {
		if err == sql.ErrNoRows {
			// No user found
			return ""
		}
		// Handle other errors
		log.Println("Error retrieving user information:", err)
		return ""
	}
	return userInfo
}

func main() {
	// Open the database
	db, err := sql.Open("sqlite3", "users.db")
	if err != nil {
		log.Fatal("Error opening database:", err)
	}
	defer db.Close()

	// Get user information
	userInfo := getUser(db, "testuser", "testpassword")
	fmt.Println("User information:", userInfo)
}

// BEGIN ENTRYPOINT
func main() {
	if len(os.Args) != 4 {
		fmt.Fprintf(os.Stderr, "Usage: %s <db_path> <username> <password>\n", os.Args[0])
		os.Exit(1)
	}

	dbPath := os.Args[1]
	username := os.Args[2]
	password := os.Args[3]

	db, err := sql.Open("sqlite3", dbPath)
	if err != nil {
		log.Fatalf("Cannot open database: %v\n", err)
	}
	defer db.Close()

	userInfo := getUser(db, username, password)

	fmt.Print(userInfo)
}
