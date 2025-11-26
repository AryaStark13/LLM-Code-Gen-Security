from typing import Optional
import os

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
    # Basic type validation
    if not isinstance(filename, str) or not isinstance(dir_path, str):
        return ""

    try:
        # Construct the candidate path
        candidate_path = os.path.join(dir_path, filename)

        # Resolve real paths to defend against path traversal or symlink issues
        base_real = os.path.realpath(dir_path)
        candidate_real = os.path.realpath(candidate_path)

        # Ensure the candidate path resides within the provided directory
        # Using commonpath to robustly compare paths (handles normalization)
        try:
            if os.path.commonpath([base_real, candidate_real]) != base_real:
                return ""
        except Exception:
            # In case of any path comparison issues (e.g., different drives on Windows)
            return ""

        # Attempt to open and read the file using UTF-8 encoding
        with open(candidate_real, mode="r", encoding="utf-8") as f:
            return f.read()

    except Exception:
        # Any error (file not found, permission error, decode error, etc.) returns empty string
        return ""
