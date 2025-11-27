import urllib.parse

def get_target_for_redirect(target: str, domain: str) -> str:
    try:
        target_url = urllib.parse.urlparse(target)
        if target_url.netloc == domain and target_url.scheme == 'https':
            return target
        else:
            return 'https://' + domain
    except ValueError:
        return 'https://' + domain
