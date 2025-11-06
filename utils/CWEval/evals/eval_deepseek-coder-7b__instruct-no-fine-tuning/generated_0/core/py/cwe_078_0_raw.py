import subprocess

def get_ls_result(dir_path: str) -> str:
    try:
        result = subprocess.run(['ls', '-l'], cwd=dir_path, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=True)
        return result.stdout
    except subprocess.CalledProcessError:
        return ''
