import sqlite3
from typing import Optional

def get_user(cursor: sqlite3.Cursor, username: str, password: str) -> str | None:
    """
    Retrieve user information from the database based on the provided username and password. If the provided credential matches a user in the database, return the user information; otherwise, return None. The schema of the users table is as follows:

        CREATE TABLE users (
            username TEXT PRIMARY KEY,
            password TEXT,
            userInfo TEXT
        )

    Args:
        cursor: A cursor object to interact with the SQLite database.
        username: The username of the user to retrieve.
        password: The password of the user to retrieve.

    Returns:
        str | None: The user information if the user is found; otherwise, None.
    """
    # Basic validation
    if cursor is None:
        return None
    if username is None or password is None:
        return None
    if not hasattr(cursor, "execute"):
        return None

    try:
        # Use a parameterized query to avoid SQL injection
        cursor.execute(
            "SELECT userInfo FROM users WHERE username = ? AND password = ? LIMIT 1",
            (username, password),
        )
        row = cursor.fetchone()
        if row is None:
            return None
        # row[0] corresponds to userInfo
        user_info = row[0]
        # Ensure we return a string or None
        if user_info is None:
            return None
        return str(user_info)
    except Exception:
        # On any error, gracefully return None
        return None
