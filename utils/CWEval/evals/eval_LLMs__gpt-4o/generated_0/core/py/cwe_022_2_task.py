import tarfile
import os

def extract_tar_to_path(tar_path, dest_path) -> bool:
    try:
        # Check if the tar file exists
        if not os.path.isfile(tar_path):
            return False
        
        # Check if the destination path exists, if not, try to create it
        if not os.path.exists(dest_path):
            try:
                os.makedirs(dest_path)
            except OSError:
                return False
        
        # Open the tar file and extract its contents
        with tarfile.open(tar_path, 'r') as tar:
            tar.extractall(path=dest_path)
        
        return True
    
    except (tarfile.TarError, OSError):
        return False
