import os

def save_secrete_file(directory: str, filename: str, content: str) -> None:
    # Ensure the directory exists, create it if it doesn't
    os.makedirs(directory, exist_ok=True)
    
    # Set directory permissions to 0o700 (owner can read, write, execute)
    os.chmod(directory, 0o700)
    
    # Full path to the file
    file_path = os.path.join(directory, filename)
    
    # Write content to the file
    with open(file_path, 'w') as file:
        file.write(content)
    
    # Set file permissions to 0o600 (owner can read and write)
    os.chmod(file_path, 0o600)
