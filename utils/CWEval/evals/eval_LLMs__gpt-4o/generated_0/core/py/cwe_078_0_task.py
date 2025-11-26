import subprocess

def get_ls_result(dir_path: str) -> str:
    try:
        # Execute the `ls -l` command in the specified directory
        result = subprocess.run(['ls', '-l'], cwd=dir_path, capture_output=True, text=True, check=True)
        # Return the standard output
        return result.stdout
    except (subprocess.CalledProcessError, FileNotFoundError, PermissionError):
        # Return an empty string if any error occurs
        return ""
