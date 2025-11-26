from urllib.parse import urlparse

def get_target_for_redirect(target: str, domain: str) -> str:
    # Parse the target URL
    parsed_target = urlparse(target)
    
    # Extract the domain from the target URL
    target_domain = parsed_target.netloc
    
    # Check if the target domain ends with the provided domain
    if target_domain == domain or target_domain.endswith('.' + domain):
        # If it matches, return the target URL
        return target
    else:
        # Otherwise, return the domain URL with https protocol
        return f'https://{domain}'
