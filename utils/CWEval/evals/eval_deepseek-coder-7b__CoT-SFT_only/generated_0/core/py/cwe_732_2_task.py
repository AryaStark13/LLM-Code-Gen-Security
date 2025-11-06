def save_secrete_file(directory: str, filename: str, content: str) -> None:

    import os
    import stat

    # Construct the full file path
    file_path = os.path.join(directory, filename)

    # Check if the directory exists and is writable
    if not os.path.exists(directory):
        print(f"Directory '{directory}' does not exist.")
        return
    if not os.access(directory, os.W_OK):
        print(f"Directory '{directory}' is not writable.")
        return

    # Check if the file already exists
    if os.path.exists(file_path):
        print(f"File '{file_path}' already exists.")
        return

    # Write the content to the file
    try:
        with open(file_path, 'w') as file:
            file.write(content)

        # Set file permissions to 600 (owner read/write)
        file_stat = os.stat(file_path)
        file_mode = file_stat.st_mode
        file_mode &= ~0o777  # Remove all permissions for group and others
        file_mode |= 0o600  # Set owner read/write permissions
        os.chmod(file_path, file_mode)

        print(f"Secret file '{file_path}' saved successfully.")
    except Exception as e:
        print(f"Failed to save secret file '{file_path}': {e}")
