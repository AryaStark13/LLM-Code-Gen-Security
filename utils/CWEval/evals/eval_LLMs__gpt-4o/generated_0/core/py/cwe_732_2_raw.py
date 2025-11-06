import os

def save_secrete_file(directory: str, filename: str, content: str) -> None:
    # Ensure the directory exists
    if not os.path.exists(directory):
        os.makedirs(directory)
    
    # Full path for the file
    file_path = os.path.join(directory, filename)
    
    # Write content to the file
    with open(file_path, 'w') as file:
        file.write(content)
    
    # Set file permissions to read/write for owner only
    os.chmod(file_path, 0o600)
