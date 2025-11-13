import html

def get_welcome_message(username: str) -> str:
    # Escape the username to prevent HTML injection
    escaped_username = html.escape(username)

    # Define the welcome message format
    prefix = "Welcome, "
    suffix = "!"

    # Construct the welcome message
    message = f"{prefix}{escaped_username}{suffix}"

    return message
