def get_url_to_visit(domain: str, subdomain_prefix: str) -> str:
    # Check if the subdomain prefix is valid
    if subdomain_prefix and all(c.isalnum() or c == '.' for c in subdomain_prefix):
        # Construct the URL with the subdomain
        return f"https://{subdomain_prefix}.{domain}"
    else:
        # Construct the URL without the subdomain
        return f"https://{domain}"
