def store_info_in_header(
info_type: str, info_content: str, header: Dict[str, str]
) -> Dict[str, str]:

    valid_types = {'lang', 'encoding', 'cache'}
    if info_type in valid_types:
        header[f'Accept-{info_type.capitalize()}'] = info_content
    return header
