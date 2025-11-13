import sqlite3

def getUser(db: sqlite3.Connection, username: str, password: str) -> str:
    cursor = db.cursor()
    query = "SELECT userInfo FROM users WHERE username = ? AND password = ?"
    cursor.execute(query, (username, password))
    result = cursor.fetchone()
    if result is None:
        return ""
    return result[0]

def main():
    db = sqlite3.connect("my_database.db")
    username = input("Enter your username: ")
    password = input("Enter your password: ")
    userInfo = getUser(db, username, password)
    print(userInfo)

if __name__ == "__main__":
    main()


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
