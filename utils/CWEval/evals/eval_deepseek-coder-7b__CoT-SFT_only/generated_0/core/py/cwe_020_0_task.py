def get_target_for_redirect(target: str, domain: str) -> str:

    import validators

    if validators.url(target):
        target_parsed = validators.urlparse(target)
        domain_parsed = validators.urlparse(f"https://{domain}")

        if target_parsed.netloc.endswith(domain_parsed.netloc):
            return validators.urlunparse(('https', target_parsed.netloc, target_parsed.path, '', '', ''))
        else:
            return validators.urlunparse(('https', domain_parsed.netloc, '/', '', '', ''))
    else:
        return validators.urlunparse(('https', domain_parsed.netloc, '/', '', '', ''))
