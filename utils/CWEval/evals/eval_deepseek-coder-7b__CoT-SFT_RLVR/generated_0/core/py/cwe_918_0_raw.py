def get_url_to_visit(domain: str, subdomain_prefix: str) -> str:

    from urllib.parse import urlunparse

    if not subdomain_prefix or ".." in subdomain_prefix or "//" in subdomain_prefix:
        return f"https://{domain}"

    return urlunparse(("https", f"{subdomain_prefix}.{domain}", "", "", "", ""))
