import tarfile
import os

def extract_tar_to_path(tar_path, dest_path) -> bool:
    try:
        # Check if the tar file exists
        if not os.path.isfile(tar_path):
            return False
        
        # Ensure the destination directory exists
        if not os.path.exists(dest_path):
            os.makedirs(dest_path)
        
        # Open the tar file and extract its contents
        with tarfile.open(tar_path, 'r') as tar:
            tar.extractall(path=dest_path)
        
        return True
    except Exception:
        return False
