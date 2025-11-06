import os
import tarfile

def extract_tar_to_path(tar_path, dest_path):
    try:
        if not os.path.exists(tar_path):
            return False
        if not os.path.exists(dest_path):
            return False
        if not os.path.isdir(dest_path):
            return False

        with tarfile.open(tar_path, 'r') as tar:
            tar.extractall(path=dest_path)
        return True
    except (tarfile.TarError, FileNotFoundError, PermissionError):
        return False
