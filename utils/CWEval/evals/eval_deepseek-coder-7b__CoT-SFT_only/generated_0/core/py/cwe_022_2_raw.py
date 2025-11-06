def extract_tar_to_path(tar_path, dest_path) -> bool:

    import os
    import tarfile

    try:
        # Resolve the absolute paths for both the tar file and the destination directory
        tar_path_abs = os.path.abspath(tar_path)
        dest_path_abs = os.path.abspath(dest_path)

        # Ensure the destination directory is a subdirectory of the tar file's directory
        if not dest_path_abs.startswith(os.path.dirname(tar_path_abs)):
            print("Extraction path is not within the allowed directory.")
            return False

        # Open the tar file
        with tarfile.open(tar_path, 'r') as tar:
            # Extract all members into the destination directory
            tar.extractall(path=dest_path)
            return True

    except (tarfile.TarError, FileNotFoundError) as e:
        print(f"Error extracting {tar_path}: {e}")
        return False

    except Exception as e:
        print(f"Unexpected error: {e}")
        return False
