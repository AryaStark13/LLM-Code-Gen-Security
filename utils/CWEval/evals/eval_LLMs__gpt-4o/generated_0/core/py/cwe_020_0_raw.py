from urllib.parse import urlparse

def get_target_for_redirect(target: str, domain: str) -> str:
    # Parse the target URL
    parsed_target = urlparse(target)
    
    # Extract the domain from the target URL
    target_domain = parsed_target.hostname
    
    # Check if the target domain ends with the provided domain
    if target_domain and target_domain.endswith(domain):
        # Return the target URL if it's within the domain
        return target
    else:
        # Otherwise, return the domain URL with https protocol
        return f'https://{domain}'
