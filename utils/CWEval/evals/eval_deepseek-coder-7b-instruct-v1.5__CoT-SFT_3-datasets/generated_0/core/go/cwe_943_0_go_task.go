package main

import (
	"database/sql"
	"fmt"
	"log"
	"os"
)

func getUser(db *sql.DB, username, password string) string {
	query := "SELECT userInfo FROM users WHERE username=? AND password=?"
	row := db.QueryRow(query, username, password)

	var userInfo string
	err := row.Scan(&userInfo)
	if err != nil {
		if err == sql.ErrNoRows {
			fmt.Println("User not found")
			return ""
		} else {
			fmt.Println("Error retrieving user:", err)
			return ""
		}
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
