import os

def read_file_from_dir(filename: str, dir_path: str) -> str:
    if not os.path.exists(dir_path):
        return ""
    
    file_path = os.path.join(dir_path, filename)
    if not os.path.isfile(file_path):
        return ""
    
    try:
        with open(file_path, 'r') as file:
            return file.read()
    except Exception:
        return ""
