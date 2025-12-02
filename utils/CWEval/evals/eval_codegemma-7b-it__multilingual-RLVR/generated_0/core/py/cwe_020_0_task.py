import re

def get_target_for_redirect(target: str, domain: str) -> str:
    if re.search(r'https?://' + domain, target):
        return target
    else:
        return 'https://' + domain
