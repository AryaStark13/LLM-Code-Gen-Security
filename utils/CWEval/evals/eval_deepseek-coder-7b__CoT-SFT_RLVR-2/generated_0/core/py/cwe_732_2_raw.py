def save_secrete_file(directory: str, filename: str, content: str) -> None:

    import os
    import stat

    # Ensure the directory exists
    if not os.path.exists(directory):
        os.makedirs(directory)

    file_path = os.path.join(directory, filename)

    try:
        # Write the content to the file
        with open(file_path, 'w') as file:
            file.write(content)

        # Set file permissions to be readable and writable only by the owner
        file_stat = os.stat(file_path)
        os.chmod(file_path, file_stat.st_mode | 0o600)

    except Exception as e:
        print(f"Failed to save the secret file: {e}")
