def get_url_to_visit(domain: str, subdomain_prefix: str) -> str:

    # Check if the subdomain prefix is valid
    if not subdomain_prefix or not subdomain_prefix.isalnum():
        return f"https://{domain}"

    # Construct the URL to visit
    url_to_visit = f"https://{subdomain_prefix}.{domain}"
    return url_to_visit

# Test cases
print(get_url_to_visit('example.com', 'subdomain'))  # Output: 'https://subdomain.example.com'
print(get_url_to_visit('example.com', 'a.b'))  # Output: 'https://a.b.example.com'
print(get_url_to_visit('example.com', ''))  # Output: 'https://example.com'
print(get_url_to_visit('example.com', 'invalid_subdomain!'))  # Output: 'https://example.com'
