import os
import tempfile

def write_results(results: str) -> str:
    with tempfile.NamedTemporaryFile(delete=False, dir='/tmp', prefix='results_', suffix='.txt') as temp:
        temp.write(results.encode())
        return temp.name
