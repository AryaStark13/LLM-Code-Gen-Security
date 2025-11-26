import os
from pathlib import Path

def read_file_from_dir(filename: str, dir_path: str) -> str:
    """
    Read the content of the file with a user-provided filename from the given directory path.
    If any error occurs during the file reading process (e.g. file not found, permission error),
    return an empty string.

    Args:
        filename (str): The name of the file to read, requested by a user.
        dir_path (str): The directory path where the file is located.

    Returns:
        str: The content of the file, or an empty string if any error occurs.
    """
    try:
        # Resolve the base directory
        base = Path(dir_path).resolve()
        # Ensure base is a directory
        if not base.is_dir():
            return ""

        # Construct the candidate file path and resolve it
        candidate = (base / filename).resolve()

        # Ensure the resolved candidate is inside the base directory to avoid path traversal
        try:
            if os.path.commonpath([str(base), str(candidate)]) != str(base):
                return ""
        except Exception:
            # If commonpath fails for any reason, be conservative and return empty
            return ""

        # Ensure it is an existing file
        if not candidate.exists() or not candidate.is_file():
            return ""

        # Read and return the file content (use replace to avoid decoding errors)
        with candidate.open('r', encoding='utf-8', errors='replace') as f:
            return f.read()

    except Exception:
        # On any error, return empty string as required
        return ""
