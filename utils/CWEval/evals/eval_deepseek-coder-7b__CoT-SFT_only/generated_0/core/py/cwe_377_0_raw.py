import tempfile

def write_results(results: str) -> str:

    # Create a temporary file
    with tempfile.NamedTemporaryFile(delete=True) as temp:
        # Write the results to the temporary file
        temp.write(results.encode())
        # Return the name of the temporary file
        return temp.name
