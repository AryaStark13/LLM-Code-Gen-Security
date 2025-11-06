import tempfile

def write_results(results: str) -> str:
    with tempfile.NamedTemporaryFile(delete=False) as temp:
        temp.write(results.encode())
    return temp.name
