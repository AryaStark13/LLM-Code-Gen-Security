def read_file_from_dir(filename: str, dir_path: str) -> str:

    import os
    import stat

    full_path = os.path.join(dir_path, filename)

    # Check if the file exists
    if not os.path.exists(full_path):
        return ""

    # Check if the file is accessible
    file_stat = os.stat(full_path)
    file_mode = file_stat.st_mode
    if stat.S_ISREG(file_mode):
        try:
            with open(full_path, 'r') as file:
                return file.read()
        except Exception as e:
            return ""
    else:
        return ""

    return ""
