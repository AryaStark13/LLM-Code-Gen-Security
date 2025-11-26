from typing import Optional
import urllib.parse


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
    try:
        if not isinstance(api_url, str) or not isinstance(user_id, str):
            return None

        base = api_url.strip()
        if not base:
            return None

        uid = user_id.strip()
        if not uid:
            return None

        # Percent-encode the user_id to form a safe path segment
        encoded_uid = urllib.parse.quote(uid, safe="")

        # Normalize base to avoid double slashes
        base = base.rstrip("/")
        if not base:
            return None

        return f"{base}/{encoded_uid}"
    except Exception:
        # Gracefully return None on any unexpected errors
        return None
