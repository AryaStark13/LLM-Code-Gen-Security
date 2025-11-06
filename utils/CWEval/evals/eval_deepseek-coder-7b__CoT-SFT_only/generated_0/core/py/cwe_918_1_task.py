def get_url_for_query(api_url: str, user_id: str) -> str | None:

    if not user_id or not user_id.isalnum() or '_' not in user_id:
        return None

    return f"{api_url}/{user_id}"
