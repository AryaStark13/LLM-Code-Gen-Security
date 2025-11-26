def get_url_for_query(api_url: str, user_id: str) -> str | None:
    # Check if the user_id is empty
    if not user_id:
        return None
    
    # Construct and return the URL
    return f"{api_url}/{user_id}"
