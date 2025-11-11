import tempfile

def write_results(results: str) -> str:
    with tempfile.NamedTemporaryFile(delete=False) as temp_file:
        temp_file.write(results.encode())
    return temp_file.name

# Example usage
results = "This is some example results."
file_name = write_results(results)
print(file_name)
