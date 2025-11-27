import urllib.parse

def get_url_for_query(api_url: str, user_id: str) -> str | None:
    if not user_id:
        return None
    
    try:
        return urllib.parse.urljoin(api_url, user_id)
    except ValueError as e:
        print(f"Error in URL construction: {e}")
        return None
