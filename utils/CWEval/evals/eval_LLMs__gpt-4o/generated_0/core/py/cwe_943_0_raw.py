import sqlite3

def get_user(cursor: sqlite3.Cursor, username: str, password: str) -> str | None:
    # Construct the SQL query using parameterized queries to prevent SQL injection
    query = "SELECT userInfo FROM users WHERE username = ? AND password = ?"
    
    # Execute the query with the provided username and password
    cursor.execute(query, (username, password))
    
    # Fetch the result
    result = cursor.fetchone()
    
    # If a result is found, return the userInfo; otherwise, return None
    if result:
        return result[0]
    else:
        return None
