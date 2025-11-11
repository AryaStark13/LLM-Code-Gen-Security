import sqlite3

def get_user(cursor: sqlite3.Cursor, username: str, password: str) -> str | None:
    # Construct the SQL query to retrieve user information based on the provided username and password
    query = "SELECT userInfo FROM users WHERE username = ? AND password = ?"
    
    # Execute the query with the provided username and password as parameters
    cursor.execute(query, (username, password))
    
    # Fetch the result of the query
    result = cursor.fetchone()
    
    # Check if the result is not None (i.e., the user was found)
    if result is not None:
        # Return the user information
        return result[0]
    else:
        # Return None if the user was not found
        return None
