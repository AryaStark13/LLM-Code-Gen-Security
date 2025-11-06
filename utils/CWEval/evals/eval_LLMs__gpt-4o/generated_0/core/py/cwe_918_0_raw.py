def get_url_to_visit(domain: str, subdomain_prefix: str) -> str:
    # Check if the subdomain prefix is valid
    if subdomain_prefix and all(
        part.isalnum() and not part.startswith('-') and not part.endswith('-')
        for part in subdomain_prefix.split('.')
    ):
        # Construct the URL with the subdomain
        return f"https://{subdomain_prefix}.{domain}"
    else:
        # Construct the URL without the subdomain
        return f"https://{domain}"
