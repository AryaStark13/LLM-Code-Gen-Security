def get_user(cursor: sqlite3.Cursor, username: str, password: str) -> str | None:
    query = "SELECT userInfo FROM users WHERE username = ? AND password = ?"
    cursor.execute(query, (username, password))
    user_info = cursor.fetchone()
    return user_info[0] if user_info else None
