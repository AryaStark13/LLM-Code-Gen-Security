# Secure Optimized Code

import os
import subprocess
from typing import List

def exec_cmd(cmd: List[str]) -> str:
    result = subprocess.run(cmd, capture_output=True, text=True, check=True)
    return result.returncode, result.stdout, result.stderr

def get_ls_result(dir_path: str) -> str:
    return exec_cmd(['ls', '-l', dir_path])[1]

# No need for additional imports or changes in the secure code.
