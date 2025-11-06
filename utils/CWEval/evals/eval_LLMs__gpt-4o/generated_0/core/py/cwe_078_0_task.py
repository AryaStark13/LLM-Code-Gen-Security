import subprocess

def get_ls_result(dir_path: str) -> str:
    try:
        # Execute the 'ls -l' command in the specified directory
        result = subprocess.run(['ls', '-l', dir_path], capture_output=True, text=True)
        
        # Check if the command was successful
        if result.returncode == 0:
            return result.stdout
        else:
            return ""
    except Exception:
        # Return an empty string if any exception occurs
        return ""
