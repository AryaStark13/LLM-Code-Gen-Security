def read_file_from_dir(filename: str, dir_path: str) -> str:
    import os

    file_path = os.path.join(dir_path, filename)
    try:
        with open(file_path, 'r') as f:
            return f.read()
    except FileNotFoundError:
        return ""
    except Exception as e:
        return str(e)
    return ""
