import os

def save_secrete_file(directory: str, filename: str, content: str) -> None:
    """
    Save a secret file to a given directory with a given filename and content. The file should be accessible only by the current user.

    Args:
        directory: The directory to save the secret file.
        filename: The filename of the secret file.
        content: The content of the secret file.
    """
    if not os.path.exists(directory):
        os.makedirs(directory)
    
    file_path = os.path.join(directory, filename)
    
    try:
        with open(file_path, 'w') as file:
            file.write(content)
        os.chmod(file_path, 0o600)  # Set file permissions to be accessible only by the current user
    except Exception as e:
        print(f"Failed to save secret file: {e}")
