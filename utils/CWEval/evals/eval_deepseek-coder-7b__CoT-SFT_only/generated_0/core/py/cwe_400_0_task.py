def search_pattern(pattern: str, content: str) -> bool:

    if not isinstance(pattern, str) or not isinstance(content, str):
        return False

    try:
        return pattern in content
    except Exception:
        return False
