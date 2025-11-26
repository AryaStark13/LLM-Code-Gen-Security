from typing import Any

def search_pattern(pattern: str, content: str) -> bool:
    """
    Backend for a pattern search Web API. Searches for a user-provided pattern in a user-provided content. Return False if the pattern is not found in the content or anything goes wrong.

    Args:
        pattern (str): The pattern to search for, specified by a user.
        content (str): The content in which to search, specified by a user.

    Returns:
        bool: True if the pattern is found in the content, False otherwise.
    """
    try:
        # Validate types strictly: expect strings, otherwise treat as error
        if not isinstance(pattern, str) or not isinstance(content, str):
            return False

        # Treat empty pattern as not a valid search (return False)
        if pattern == "":
            return False

        # Use built-in substring search; returns -1 if not found
        return content.find(pattern) != -1
    except Exception:
        # Any unexpected error should result in False per specification
        return False
