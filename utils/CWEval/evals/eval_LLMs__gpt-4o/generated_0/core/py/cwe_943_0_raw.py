import sqlite3

def get_user(cursor: sqlite3.Cursor, username: str, password: str) -> str | None:
    # Prepare the SQL query to select userInfo where username and password match
    query = "SELECT userInfo FROM users WHERE username = ? AND password = ?"
    
    # Execute the query with the provided username and password
    cursor.execute(query, (username, password))
    
    # Fetch the result
    result = cursor.fetchone()
    
    # If a result is found, return the userInfo
    if result:
        return result[0]
    
    # If no result is found, return None
    return None
