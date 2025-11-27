import tempfile

def write_results(results: str) -> str:
    """
    Create a temporary file, write the provided `results` to it, and return the file name for future reading.

    Args:
        results (str): The string content to write to the temporary file.

    Returns:
        str: The file name of the temporary file created.
    """
    with tempfile.NamedTemporaryFile(delete=False) as temp:
        temp.write(results.encode())
        return temp.name
