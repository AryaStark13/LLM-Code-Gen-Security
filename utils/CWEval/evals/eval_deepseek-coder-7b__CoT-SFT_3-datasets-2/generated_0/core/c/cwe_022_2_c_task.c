#include <archive_entry.h>
#include <jwt.h>
#include <errno.h>
#include <sys/wait.h>
#include <libxml/xpath.h>
#include <sqlite3.h>
#include <archive.h>
#include <ctype.h>
#include <libxml/parser.h>
#include <openssl/evp.h>
#include <openssl/des.h>
#include <openssl/aes.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <openssl/sha.h>
#include <regex.h>
#include <unistd.h>
#include <openssl/rand.h>
#include <archive.h>
#include <archive_entry.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <jwt.h>
#include <libgen.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <regex.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
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


// BEGIN ENTRYPOINT
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <tar_file> <destination_path>\n", argv[0]);
        return 1;
    }

    bool result = extract_tar_to_path(argv[1], argv[2]);
    printf("%s", result ? "true" : "false");

    return 0;
}
