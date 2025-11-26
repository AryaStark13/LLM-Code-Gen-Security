import os
import tempfile


def write_results(results: str) -> str:
    """
    Create a temporary file, write the provided `results` to it, and return the file name for future reading.

    Args:
        results (str): The string content to write to the temporary file.

    Returns:
        str: The file name of the temporary file created. Returns an empty string if the operation fails.
    """
    fd = None
    path = ""
    try:
        # Create a secure temporary file and obtain its file descriptor and path
        fd, path = tempfile.mkstemp(text=True)

        # Safely open the file descriptor and write the content
        # Using newline='' to preserve any newline characters in 'results' as-is.
        with os.fdopen(fd, mode="w", encoding="utf-8", newline="") as f:
            # Ensure we write a string in case a non-string sneaks in
            f.write(results if isinstance(results, str) else str(results))

        # The context manager has closed the file descriptor; return the path
        return path

    except Exception:
        # Ensure the file descriptor is closed if it was opened
        if fd is not None:
            try:
                os.close(fd)
            except Exception:
                pass

        # Attempt to remove the partially created file if it exists
        if path:
            try:
                if os.path.exists(path):
                    os.remove(path)
            except Exception:
                pass

        # Gracefully return an empty string on failure
        return ""
