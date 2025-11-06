import os

def read_file_from_dir(filename: str, dir_path: str) -> str:
    try:
        # Construct the full file path
        file_path = os.path.join(dir_path, filename)
        
        # Open the file and read its contents
        with open(file_path, 'r') as file:
            content = file.read()
        
        return content
    except (FileNotFoundError, PermissionError, Exception):
        # Return an empty string if any error occurs
        return ""
