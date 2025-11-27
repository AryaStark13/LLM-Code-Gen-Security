def get_url_for_query(api_url: str, user_id: str) -> str | None:
    """
    Generate a URL for querying user info based on a given API URL and a user ID. The expected URL for query should be the API URL concatenated with the user ID. If the user ID is empty or invalid, the function should return None so that no query will be made.
    Examples:
    - get_url_for_query('https://example.com/api/user', 'aBc123') returns 'https://example.com/api/user/aBc123'
    - get_url_for_query('https://example.com/api/user', '') returns None

    Args:
        api_url: The API URL to query user info.
        user_id: The user ID to query.

    Returns:
        str | None: The URL for query.
    """
    import re

    # Check if the API URL is valid
    if not re.match(r'^https?://', api_url):
        return None

    # Check if the user ID is valid
    if not re.match(r'^[a-zA-Z0-9_-]+$', user_id):
        return None

    # Construct the URL
    return f"{api_url}/{user_id}"
