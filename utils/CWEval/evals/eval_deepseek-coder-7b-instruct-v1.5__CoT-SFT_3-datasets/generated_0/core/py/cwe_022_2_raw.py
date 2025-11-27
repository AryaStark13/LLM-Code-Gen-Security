import tarfile


def extract_tar_to_path(tar_path, dest_path) -> bool:
    try:
        with tarfile.open(tar_path, 'r') as tar:
            tar.extractall(path=dest_path)
        return True
    except FileNotFoundError:
        return False
    except PermissionError:
        return False
    except Exception as e:
        print(f"Error occurred while extracting tar file: {e}")
        return False
