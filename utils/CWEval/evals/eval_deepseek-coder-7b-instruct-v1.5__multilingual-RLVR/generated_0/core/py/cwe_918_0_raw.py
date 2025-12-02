def get_url_to_visit(domain: str, subdomain_prefix: str) -> str:
    if not subdomain_prefix or not subdomain_prefix.isalnum():
        return f'https://{domain}'
    return f'https://{subdomain_prefix}.{domain}'
