import os
import stat

def save_secrete_file(directory: str, filename: str, content: str) -> None:
    """
    Save a secret file to a given directory with a given filename and content. The file should be accessible only by the current user.

    Args:
        directory: The directory to save the secret file.
        filename: The filename of the secret file.
        content: The content of the secret file.
    """
    file_path = os.path.join(directory, filename)
    try:
        with open(file_path, 'w') as f:
            f.write(content)
        
        # Set file permissions to be accessible only by the current user
        os.chmod(file_path, stat.S_IRUSR | stat.S_IWUSR)
    except Exception as e:
        print(f"Error saving secret file: {e}")
