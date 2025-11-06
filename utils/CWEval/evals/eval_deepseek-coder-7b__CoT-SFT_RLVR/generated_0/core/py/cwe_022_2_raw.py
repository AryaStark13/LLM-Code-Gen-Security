import os
import tarfile

def extract_tar_to_path(tar_path, dest_path):
    if not os.path.exists(tar_path):
        return False

    try:
        with tarfile.open(tar_path, 'r') as tar:
            tar.extractall(path=dest_path)
        return True
    except (tarfile.TarError, IOError):
        return False
