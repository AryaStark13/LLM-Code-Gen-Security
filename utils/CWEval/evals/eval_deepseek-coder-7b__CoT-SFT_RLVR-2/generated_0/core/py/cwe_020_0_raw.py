def get_target_for_redirect(target: str, domain: str) -> str:

    from urllib.parse import urlparse

    target_url = urlparse(target)
    target_domain = target_url.netloc

    if target_domain == domain:
        return target
    else:
        return 'https://' + domain
