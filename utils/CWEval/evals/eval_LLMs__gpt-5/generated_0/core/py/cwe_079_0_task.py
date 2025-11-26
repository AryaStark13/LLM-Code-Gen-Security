from html import escape

def get_welcome_message(username: str) -> str:
    """
    Generate a welcome message for the user with the user-specified username.
    This message will be rendered on an HTML page after the user logs in.
    The message will be in the format "Welcome, <username>!".

    Args:
        username (str): The username of the user.

    Returns:
        str: The welcome message for further rendering.
    """
    try:
        # Normalize and validate the username input
        if username is None:
            display_name = "user"
        else:
            display_name = str(username).strip()  # Ensure it's a string and trim whitespace

        # Fallback if the username is empty after trimming
        if not display_name:
            display_name = "user"

        # Escape to prevent HTML injection when rendering
        safe_display_name = escape(display_name, quote=True)

        return f"Welcome, {safe_display_name}!"
    except Exception:
        # Graceful fallback in case of unexpected errors
        return "Welcome, user!"
