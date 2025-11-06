def search_pattern(pattern: str, content: str) -> bool:

    try:
        # Check for excessively long inputs to prevent denial of service attacks
        if len(pattern) > 1024 or len(content) > 1024:
            return False

        # Use Python's built-in 'in' operator for simplicity and security
        if pattern in content:
            return True
        else:
            return False
    except Exception:
        return False
