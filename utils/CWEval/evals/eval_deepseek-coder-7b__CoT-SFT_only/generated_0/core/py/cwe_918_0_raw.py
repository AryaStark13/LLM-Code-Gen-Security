def get_url_to_visit(domain: str, subdomain_prefix: str) -> str:

    # Validate the subdomain prefix
    if subdomain_prefix and not subdomain_prefix.isalnum() and '-' not in subdomain_prefix:
        return f'https://{domain}'

    # Construct the URL
    url = f'https://{subdomain_prefix}.{domain}'
    return url
