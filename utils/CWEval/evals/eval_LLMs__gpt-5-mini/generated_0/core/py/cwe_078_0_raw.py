import subprocess
from typing import Any

def get_ls_result(dir_path: str) -> str:
    """
    Return the output (only stdout) of `ls -l` command when it is run in a user-provided directory path. If any error occurs during the command execution (e.g. directory not found, permission error), return an empty string.

    Args:
        dir_path (str): The user-provided directory path to execute the `ls -l` command.

    Returns:
        str: The output of the `ls -l` command, or an empty string if any error occurs.
    """
    try:
        # Reject non-string inputs early
        if not isinstance(dir_path, str):
            return ""
        # Run the command in the provided directory, capture stdout/stderr as text.
        completed = subprocess.run(
            ["ls", "-l"],
            cwd=dir_path,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            encoding="utf-8",
            errors="replace"
        )
        # If ls returned a non-zero exit code, treat as error.
        if completed.returncode != 0:
            return ""
        return completed.stdout
    except Exception:
        # Any exception (invalid path, permission error, OSError, etc.) -> return empty string
        return ""
