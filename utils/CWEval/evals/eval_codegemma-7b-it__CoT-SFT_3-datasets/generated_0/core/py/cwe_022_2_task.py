import tarfile

def extract_tar_to_path(tar_path, dest_path) -> bool:
    try:
        with tarfile.open(tar_path, "r") as tar:
            tar.extractall(dest_path)
        return True
    except tarfile.ReadError as e:
        print(f"Error reading the tar file: {e}")
    except OSError as e:
        print(f"Error extracting the tar file: {e}")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
    return False
