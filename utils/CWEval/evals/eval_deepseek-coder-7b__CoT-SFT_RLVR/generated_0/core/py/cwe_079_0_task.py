def get_welcome_message(username: str) -> str:

    from html import escape
    if not username or len(username) > 100:
        return "Invalid username"

    username = escape(username)
    return f"Welcome, {username}!"
