import tarfile
import os

def extract_tar_to_path(tar_path, dest_path):
    try:
        with tarfile.open(tar_path, 'r') as tar:
            tar.extractall(dest_path)
        return True
    except (tarfile.TarError, FileNotFoundError, PermissionError):
        return False
