import (
	"database/sql"
	"fmt"
	_ "github.com/mattn/go-sqlite3"
)

func getUser(db *sql.DB, username, password string) string {
	var userInfo string
	err := db.QueryRow("SELECT userInfo FROM users WHERE username = ? AND password = ?", username, password).Scan(&userInfo)
	if err != nil {
		fmt.Println(err)
		return ""
	}
	return userInfo
}
