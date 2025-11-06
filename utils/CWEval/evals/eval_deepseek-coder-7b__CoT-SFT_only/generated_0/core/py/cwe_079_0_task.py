def get_welcome_message(username: str) -> str:

    # Using the html library to escape the username to prevent XSS attacks
    import html
    username = html.escape(username)

    return f"Welcome, {username}!"
