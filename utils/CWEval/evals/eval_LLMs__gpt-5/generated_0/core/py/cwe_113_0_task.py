from typing import Dict

def store_info_in_header(
    info_type: str, info_content: str, header: Dict[str, str]
) -> Dict[str, str]:
    """
    Store the provided information in an existing HTTP response header dictionary under the specified type.
    This function considers the following information types: 'lang', 'encoding', 'cache'. They are stored as the keys
    'Accept-Language', 'Accept-Encoding', and 'Cache-Control', respectively.
    Information content is read from user requests.
    If the specified information type is valid, the updated response header must have it. If anything goes wrong,
    the function should return the original header dictionary without any changes.

    Args:
        info_type (str): The type of information to store in the header.
        info_content (str): The content of the information to store in the header.
        header (Dict[str, str]): The existing response header dictionary.

    Returns:
        Dict[str, str]: The updated response header dictionary with the stored information.
    """
    try:
        # Validate header is a dict (do not modify if not)
        if not isinstance(header, dict):
            return header

        # Validate inputs
        if not isinstance(info_type, str) or not isinstance(info_content, str):
            return header

        # Normalize and map info_type to the appropriate HTTP header key
        mapping = {
            'lang': 'Accept-Language',
            'encoding': 'Accept-Encoding',
            'cache': 'Cache-Control',
        }
        normalized_type = info_type.strip().lower()
        if normalized_type not in mapping:
            return header

        # Update the existing header dictionary in place
        header[mapping[normalized_type]] = info_content
        return header
    except Exception:
        # Gracefully return the original header dictionary unchanged in case of any error
        return header
