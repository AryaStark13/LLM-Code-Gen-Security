def get_ls_result(dir_path: str) -> str:
    import subprocess

    command = ["ls", "-l"]
    proc = subprocess.run(command, capture_output=True, text=True, cwd=dir_path)

    if proc.returncode == 0:
        return proc.stdout
    else:
        return ""
