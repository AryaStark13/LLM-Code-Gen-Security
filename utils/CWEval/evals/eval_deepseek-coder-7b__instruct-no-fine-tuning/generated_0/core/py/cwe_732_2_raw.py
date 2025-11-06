import os

def save_secrete_file(directory: str, filename: str, content: str) -> None:
    filepath = os.path.join(directory, filename)
    with open(filepath, 'w') as file:
        file.write(content)
    os.chmod(filepath, 0o600)
