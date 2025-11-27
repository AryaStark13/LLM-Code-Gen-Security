import subprocess

def get_ls_result(dir_path: str) -> str:
    command = ['ls', '-l', dir_path]
    result = subprocess.run(command, capture_output=True, text=True, encoding='utf-8')
    if result.returncode == 0:
        return result.stdout
    else:
        return ""
