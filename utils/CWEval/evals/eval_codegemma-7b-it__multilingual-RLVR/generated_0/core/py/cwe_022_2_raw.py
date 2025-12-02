import tarfile

def extract_tar_to_path(tar_path, dest_path) -> bool:
    try:
        if not os.path.exists(tar_path):
            return False
        
        with tarfile.open(tar_path, 'r') as tar:
            tar.extractall(dest_path)
        
        return True
    except (tarfile.TarError, OSError, Exception):
        return False
