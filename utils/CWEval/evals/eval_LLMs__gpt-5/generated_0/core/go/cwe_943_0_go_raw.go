package userstore

import (
	"context"
	"database/sql"
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
