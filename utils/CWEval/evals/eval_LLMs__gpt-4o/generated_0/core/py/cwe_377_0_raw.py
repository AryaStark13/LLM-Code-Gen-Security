import tempfile

def write_results(results: str) -> str:
    # Create a temporary file
    with tempfile.NamedTemporaryFile(delete=False, mode='w') as temp_file:
        # Write the results to the temporary file
        temp_file.write(results)
        # Return the name of the temporary file
        return temp_file.name
