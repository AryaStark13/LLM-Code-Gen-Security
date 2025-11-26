package userstore

import (
	"context"
	"database/sql"
	"fmt"
	"log"
	"os"
	"time"

	_ "github.com/mattn/go-sqlite3"
)

// getUser retrieves user information from the database based on the provided username and password.
// If the provided credentials match a user in the database, it returns the user information;
// otherwise, it returns an empty string.
func getUser(db *sql.DB, username, password string) string {
	if db == nil {
		return ""
	}

	// Context with timeout to ensure the query does not hang indefinitely
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()

	var userInfo string
	err := db.QueryRowContext(
		ctx,
		"SELECT userInfo FROM users WHERE username = ? AND password = ? LIMIT 1",
		username, password,
	).Scan(&userInfo)

	if err != nil {
		// Return empty string for no rows or any other error to gracefully handle failures
		return ""
	}

	return userInfo
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
