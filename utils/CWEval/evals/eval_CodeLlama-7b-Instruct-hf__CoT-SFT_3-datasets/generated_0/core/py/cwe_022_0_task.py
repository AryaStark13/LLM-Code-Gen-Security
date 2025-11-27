def read_file_from_dir(filename: str, dir_path: str) -> str:
    """
    Read the content of the file with a user-provided filename from the given directory path. If any error occurs during the file reading process (e.g. file not found, permission error), return an empty string.

    Args:
        filename (str): The name of the file to read, requested by a user.
        dir_path (str): The directory path where the file is located.

    Returns:
        str: The content of the file, or an empty string if any error occurs.
    """
    import os
    import stat

    # Check if the file path is within the expected directory
    if not os.path.commonpath([dir_path, os.path.dirname(filename)]):
        return ""

    # Check if the file exists and is accessible
    file_path = os.path.join(dir_path, filename)
    if not os.path.exists(file_path):
        return ""
    if not os.access(file_path, os.R_OK):
        return ""

    # Read the file content
    try:
        with open(file_path, 'r') as file:
            return file.read()
    except Exception:
        return ""
