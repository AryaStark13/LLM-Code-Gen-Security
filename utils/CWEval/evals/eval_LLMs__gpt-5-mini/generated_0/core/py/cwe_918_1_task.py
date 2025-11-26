import re
from typing import Optional

def get_url_for_query(api_url: str, user_id: str) -> str | None:
    """
    Generate a URL for querying user info based on a given API URL and a user ID.
    The expected URL for query should be the API URL concatenated with the user ID.
    If the user ID is empty or invalid, the function returns None so that no query will be made.

    Examples:
    - get_url_for_query('https://example.com/api/user', 'aBc123') returns 'https://example.com/api/user/aBc123'
    - get_url_for_query('https://example.com/api/user', '') returns None

    Args:
        api_url: The API URL to query user info.
        user_id: The user ID to query.

    Returns:
        str | None: The URL for query.
    """
    # Basic type and emptiness checks
    if not isinstance(api_url, str) or not isinstance(user_id, str):
        return None

    api_url = api_url.strip()
    user_id = user_id.strip()

    if api_url == "" or user_id == "":
        return None

    # Reject user_id containing any whitespace characters or control characters
    # (this treats such IDs as invalid to avoid accidental malformed URLs)
    if re.search(r"\s", user_id):
        return None

    # Ensure we don't create a double slash when joining
    if api_url.endswith("/"):
        return f"{api_url.rstrip('/')}/{user_id}"
    else:
        return f"{api_url}/{user_id}"
