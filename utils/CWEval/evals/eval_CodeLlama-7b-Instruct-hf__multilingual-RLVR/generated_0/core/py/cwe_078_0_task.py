def get_ls_result(dir_path: str) -> str:
    import subprocess
    try:
        result = subprocess.run(['ls', '-l', dir_path], capture_output=True, text=True).stdout
        return result.strip()
    except Exception:
        return ""
