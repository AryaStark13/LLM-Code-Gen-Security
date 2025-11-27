package main

import (
	"database/sql"
	"fmt"
	"github.com/mattn/go-sqlite3"
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
