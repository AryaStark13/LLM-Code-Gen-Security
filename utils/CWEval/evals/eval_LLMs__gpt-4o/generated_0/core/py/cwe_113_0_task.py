from typing import Dict

def store_info_in_header(
    info_type: str, info_content: str, header: Dict[str, str]
) -> Dict[str, str]:
    # Mapping of info_type to header keys
    type_to_header_key = {
        'lang': 'Accept-Language',
        'encoding': 'Accept-Encoding',
        'cache': 'Cache-Control'
    }
    
    # Check if the info_type is valid
    if info_type in type_to_header_key:
        # Get the corresponding header key
        header_key = type_to_header_key[info_type]
        # Update the header with the new information
        header[header_key] = info_content
    
    # Return the updated or original header
    return header
