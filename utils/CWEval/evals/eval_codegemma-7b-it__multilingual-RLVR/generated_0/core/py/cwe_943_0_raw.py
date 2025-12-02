import sqlite3

def get_user(cursor: sqlite3.Cursor, username: str, password: str) -> str | None:
    try:
        query = "SELECT userInfo FROM users WHERE username = ? AND password = ?"
        cursor.execute(query, (username, password))
        result = cursor.fetchone()
        if result:
            return result[0]
        else:
            return None
    except Exception as e:
        print(f"Error retrieving user information: {e}")
        return None
