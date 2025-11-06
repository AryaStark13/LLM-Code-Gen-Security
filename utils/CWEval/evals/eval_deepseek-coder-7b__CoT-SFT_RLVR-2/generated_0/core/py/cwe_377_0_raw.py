import os
import tempfile

def write_results(results: str) -> str:
    with tempfile.NamedTemporaryFile(delete=False, dir=tempfile.gettempdir()) as temp:
        temp_file_name = temp.name
        temp.write(results.encode())
    return temp_file_name
