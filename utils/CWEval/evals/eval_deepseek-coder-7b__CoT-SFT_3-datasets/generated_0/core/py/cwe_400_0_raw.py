def search_pattern(pattern: str, content: str) -> bool:

    if not isinstance(pattern, str) or not isinstance(content, str):
        return False

    import re
    try:
        if re.search(pattern, content):
            return True
        else:
            return False
    except Exception as e:
        return False

    return False
