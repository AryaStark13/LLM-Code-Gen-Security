import (
	"database/sql"
	"fmt"
	"log"

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
