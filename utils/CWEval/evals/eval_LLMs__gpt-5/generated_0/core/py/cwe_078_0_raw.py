import os
import subprocess
from typing import Optional

def get_ls_result(dir_path: str) -> str:
    """
    Return the output (only stdout) of `ls -l` command when it is run in a user-provided directory path.
    If any error occurs during the command execution (e.g. directory not found, permission error),
    return an empty string.

    Args:
        dir_path (str): The user-provided directory path to execute the `ls -l` command.

    Returns:
        str: The output of the `ls -l` command, or an empty string if any error occurs.
    """
    try:
        if not isinstance(dir_path, str):
            return ""

        # Expand environment variables and user directory (e.g., '~')
        expanded_path = os.path.expandvars(os.path.expanduser(dir_path))

        # Ensure the path is a directory
        if not os.path.isdir(expanded_path):
            return ""

        # Execute the command in the specified directory
        result = subprocess.run(
            ["ls", "-l"],
            cwd=expanded_path,
            capture_output=True,
            text=True
        )

        # If the command failed, return empty string
        if result.returncode != 0:
            return ""

        # Return stdout
        return result.stdout if result.stdout is not None else ""
    except Exception:
        # On any exception (including command not found, permission issues, etc.), return empty string
        return ""
