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
