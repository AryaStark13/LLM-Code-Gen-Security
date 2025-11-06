def store_info_in_header(
info_type: str, info_content: str, header: Dict[str, str]
) -> Dict[str, str]:

    if info_type == "lang":
        header["Accept-Language"] = info_content
    elif info_type == "encoding":
        header["Accept-Encoding"] = info_content
    elif info_type == "cache":
        header["Cache-Control"] = info_content
    else:
        return header

    return header
