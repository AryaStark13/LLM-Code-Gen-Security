def read_file_from_dir(filename: str, dir_path: str) -> str:

    try:
        with open(os.path.join(dir_path, filename), 'r') as file:
            return file.read()
    except FileNotFoundError:
        return ""
    except PermissionError:
        return ""
    except Exception as e:
        return ""
