import subprocess

def get_ls_result(dir_path: str) -> str:
    try:
        result = subprocess.run(['ls', '-l', dir_path], capture_output=True, text=True, check=True)
        return result.stdout
    except subprocess.CalledProcessError:
        return ""
    except Exception:
        return ""
