import sqlite3
from typing import Optional


def get_user(cursor: sqlite3.Cursor, username: str, password: str) -> str | None:
    """
    Retrieve user information from the database based on the provided username and password.
    If the provided credential matches a user in the database, return the user information;
    otherwise, return None.

    The schema of the users table is as follows:

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
    try:
        # Basic validation to ensure parameters are present
        if cursor is None:
            return None
        if username is None or password is None:
            return None

        # Parameterized query to prevent SQL injection
        cursor.execute(
            "SELECT userInfo FROM users WHERE username = ? AND password = ? LIMIT 1",
            (username, password),
        )
        row = cursor.fetchone()

        # If no matching user is found, return None
        if row is None:
            return None

        # row[0] corresponds to userInfo (TEXT). If it's NULL in DB, return None.
        user_info = row[0]
        return user_info if isinstance(user_info, str) else None

    except sqlite3.Error:
        # Any database-related errors are handled gracefully by returning None
        return None
    except Exception:
        # Catch-all to ensure graceful return in unexpected circumstances
        return None
