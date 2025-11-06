def read_file_from_dir(filename: str, dir_path: str) -> str:

    import os
    try:
        file_path = os.path.join(dir_path, filename)
        with open(file_path, 'r') as file:
            return file.read()
    except FileNotFoundError:
        return ""
    except PermissionError:
        return ""
    except Exception:
        return ""
