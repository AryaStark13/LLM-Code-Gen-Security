def get_url_for_query(api_url: str, user_id: str) -> str | None:
    # Check if the user ID is valid (non-empty)
    if not user_id:
        return None
    
    # Concatenate the API URL with the user ID
    return f"{api_url}/{user_id}"
