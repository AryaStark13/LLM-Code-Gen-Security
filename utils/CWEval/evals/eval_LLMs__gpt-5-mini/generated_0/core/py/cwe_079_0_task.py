import html
from typing import Any

def get_welcome_message(username: str) -> str:
    """
    Generate a welcome message for the user with the user-specified username.
    This message will be rendered in a HTML page after the user logs in.
    The message will be in the format "Welcome, <username>!".

    Args:
        username (str): The username of the user.

    Returns:
        str: The welcome message for further rendering.
    """
    try:
        # Ensure we have a string (handle None or other types gracefully)
        if username is None:
            username_str = ""
        else:
            # Convert to str in case a non-string type is passed
            username_str = str(username)

        # Escape HTML special characters to prevent XSS when rendering in HTML
        safe_username = html.escape(username_str, quote=True)

        return f"Welcome, {safe_username}!"
    except Exception:
        # In case of any unexpected error, return a safe default message
        return "Welcome, !"
