def read_file_from_dir(filename: str, dir_path: str) -> str:
    try:
        with open(f'{dir_path}/{filename}', 'r') as file:
            return file.read()
    except Exception:
        return ''
