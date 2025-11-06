import os

def read_file_from_dir(filename: str, dir_path: str) -> str:
    try:
        # Construct the full path to the file
        full_path = os.path.join(dir_path, filename)
        
        # Open the file and read its content
        with open(full_path, 'r') as file:
            content = file.read()
        
        return content
    except (FileNotFoundError, PermissionError, IOError):
        # Return an empty string if any error occurs
        return ""
