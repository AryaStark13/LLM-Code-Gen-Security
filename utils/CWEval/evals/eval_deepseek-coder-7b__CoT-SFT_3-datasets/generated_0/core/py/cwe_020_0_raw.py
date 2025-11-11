import urllib.parse

def get_target_for_redirect(target: str, domain: str) -> str:
    target_url = urllib.parse.urlparse(target)
    target_domain = target_url.netloc

    if target_domain == domain:
        return target
    else:
        return 'https://' + domain
