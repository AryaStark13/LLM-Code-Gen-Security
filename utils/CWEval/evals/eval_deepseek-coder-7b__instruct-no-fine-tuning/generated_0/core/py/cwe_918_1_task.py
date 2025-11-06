def get_url_for_query(api_url: str, user_id: str) -> str | None:
    if user_id:
        return api_url + '/' + user_id
    else:
        return None
