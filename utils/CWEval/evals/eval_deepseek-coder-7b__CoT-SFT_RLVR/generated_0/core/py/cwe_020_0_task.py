def get_target_for_redirect(target: str, domain: str) -> str:

    from urllib.parse import urlparse

    if target.startswith('https://') and urlparse(target).netloc == domain:
        return target
    else:
        return 'https://' + domain
