def get_target_for_redirect(target: str, domain: str) -> str:
    import urllib.parse

    target_url = urllib.parse.urlparse(target)
    domain_url = urllib.parse.urlparse(f"https://{domain}")

    if target_url.hostname.endswith(domain_url.hostname):
        return target
    else:
        return domain_url.geturl()
