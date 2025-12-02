package main

import (
	"database/sql"
	"fmt"
	"log"

	_ "github.com/mattn/go-sqlite3"
)

func getUser(db *sql.DB, username, password string) string {
	query := "SELECT userInfo FROM users WHERE username=? AND password=?"
	row := db.QueryRow(query, username, password)

	var userInfo string
	err := row.Scan(&userInfo)
	if err != nil {
		if err == sql.ErrNoRows {
			return ""
		}
		log.Println(err)
		return ""
	}

	return userInfo
}
