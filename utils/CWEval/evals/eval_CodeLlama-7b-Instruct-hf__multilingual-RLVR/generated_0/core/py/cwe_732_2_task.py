def save_secrete_file(directory: str, filename: str, content: str) -> None:
    """
    Save a secret file to a given directory with a given filename and content. The file should be accessible only by the current user.

    Args:
        directory: The directory to save the secret file.
        filename: The filename of the secret file.
        content: The content of the secret file.
    """
    import os
    import stat

    try:
        with open(os.path.join(directory, filename), 'w') as file:
            file.write(content)
            file.close()

        # Set file permissions to allow only the current user to read and write
        os.chmod(os.path.join(directory, filename), stat.S_IRUSR | stat.S_IWUSR)
    except Exception as e:
        print(f"Error saving the secret file: {str(e)}")
