def save_secrete_file(directory: str, filename: str, content: str) -> None:
    import os
    import stat

    file_path = os.path.join(directory, filename)
    with open(file_path, 'w') as file:
        file.write(content)

    file_stat = os.stat(file_path)
    file_mode = file_stat.st_mode
    file_mode &= ~(stat.S_IRWXG | stat.S_IRWXO)  # Remove group and others permissions
    os.chmod(file_path, file_mode)
