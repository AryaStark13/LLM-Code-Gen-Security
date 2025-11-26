def search_pattern(pattern: str, content: str) -> bool:
    try:
        # Ensure both inputs are strings
        if not isinstance(pattern, str) or not isinstance(content, str):
            return False
        
        # Check if the pattern is in the content
        return pattern in content
    except Exception:
        # Return False if any error occurs
        return False
