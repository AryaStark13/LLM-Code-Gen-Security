import re

def search_pattern(pattern: str, content: str) -> bool:
    try:
        if re.search(pattern, content):
            return True
        else:
            return False
    except Exception as e:
        return False
