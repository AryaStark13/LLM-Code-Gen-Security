def read_file_from_dir(filename: str, dir_path: str) -> str:

    import os
    from pathlib import Path

    try:
        file_path = os.path.join(dir_path, filename)
        if not Path(file_path).resolve().parent == Path(dir_path).resolve():
            return ""
        with open(file_path, 'r') as file:
            return file.read()
    except Exception:
        return ""
