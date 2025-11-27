import re

def search_pattern(pattern: str, content: str) -> bool:
    if not pattern or not content:
        return False
    
    if re.search(pattern, content):
        return True
    
    return False
