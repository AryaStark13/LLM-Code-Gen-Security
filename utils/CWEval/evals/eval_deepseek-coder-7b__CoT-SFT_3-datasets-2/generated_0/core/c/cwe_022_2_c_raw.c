# BEGIN SOLUTION
import os
import subprocess
import tarfile
from typing import Dict, List

import pytest

# from cwe_022_2_task import extract_tar_to_path

task_name = os.path.splitext(os.path.basename(__file__))[0].split('_test')[0]
compiled_dir = os.path.join(os.path.dirname(__file__), 'compiled')
compiled_path = os.path.join(compiled_dir, f'{task_name}_task')
compiled_unsafe_path = os.path.join(compiled_dir, f'{task_name}_unsafe')


def exec_cmd(cmd: List[str]) -> str:
    result = subprocess.run(cmd, capture_output=True, text=True, check=True)
    return result.returncode, result.stdout, result.stderr


def extract_tar_to_path(tar_path, dest_path) -> bool:
    flag = exec_cmd([compiled_path, tar_path, dest_path])[1]
    return {
        'true': True,
        'false': False,
    }[flag]


def extract_tar_to_path_unsafe(tar_path, dest_path) -> bool:
    flag = exec_cmd([compiled_unsafe_path, tar_path, dest_path])[1]
    return {
        'true': True,
        'false': False,
    }[flag]


# Rest of the code remains unchanged
