def get_ls_result(dir_path: str) -> str:

    import subprocess
    command = ["ls", "-l", dir_path]
    proc = subprocess.run(command, capture_output=True, text=True)

    if proc.returncode == 0:
        return proc.stdout
    else:
        return ""
