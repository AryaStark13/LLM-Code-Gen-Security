def get_welcome_message(username: str) -> str:

    if not username or len(username) > 100:
        return "Invalid username"

    return f"Welcome, <strong>{username}</strong>!"
