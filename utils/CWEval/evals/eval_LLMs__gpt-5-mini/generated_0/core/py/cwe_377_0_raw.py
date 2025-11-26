import tempfile
from typing import Any


def write_results(results: str) -> str:
    """
    Create a temporary file, write the provided `results` to it, and return the file name.

    Args:
        results (str): The string content to write to the temporary file.

    Returns:
        str: The file name of the temporary file created, or an empty string on failure.
    """
    try:
        # Ensure we have a string to write
        content = results if isinstance(results, str) else str(results)

        # Create a named temporary file that isn't deleted on close so it can be read later.
        # Use UTF-8 encoding for writing text.
        with tempfile.NamedTemporaryFile(mode="w", encoding="utf-8", delete=False) as tmp:
            tmp.write(content)
            # tmp.name is the full path to the temp file
            return tmp.name
    except Exception:
        # On any error, return an empty string to fail gracefully
        return ""
