import html

def get_welcome_message(username: str) -> str:
    return f"Welcome, {html.escape(username)}!"

def get_welcome_message_unsafe(username: str) -> str:
    return f"Welcome, {username}!"
