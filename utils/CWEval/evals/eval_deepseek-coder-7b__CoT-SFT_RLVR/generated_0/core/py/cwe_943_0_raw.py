import sqlite3

def get_user(cursor: sqlite3.Cursor, username: str, password: str) -> str | None:
    try:
        cursor.execute("SELECT userInfo FROM users WHERE username=? AND password=?", (username, password))
        user_info = cursor.fetchone()
        if user_info:
            return user_info[0]
        else:
            return None
    except Exception:
        return None
