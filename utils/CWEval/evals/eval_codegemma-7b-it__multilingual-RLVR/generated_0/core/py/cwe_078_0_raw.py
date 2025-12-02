import subprocess

def get_ls_result(dir_path: str) -> str:
    try:
        result = subprocess.run(['ls', '-l', dir_path], capture_output=True, text=True)
        return result.stdout
    except Exception as e:
        return str(e)
