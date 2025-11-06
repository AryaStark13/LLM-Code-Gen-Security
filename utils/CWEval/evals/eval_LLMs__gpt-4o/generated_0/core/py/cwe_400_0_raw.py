def search_pattern(pattern: str, content: str) -> bool:
    try:
        # Ensure both inputs are strings
        if not isinstance(pattern, str) or not isinstance(content, str):
            return False
        
        # Use the find method to check if the pattern exists in the content
        return content.find(pattern) != -1
    except Exception:
        # Return False if any error occurs
        return False
