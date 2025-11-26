# reverting back to commit: 2cbe2c39edd523c7930b27036663d9e261d22071
# this file can be seen at: https://github.com/AryaStark13/LLM-Code-Gen-Security/blob/2cbe2c39edd523c7930b27036663d9e261d22071/utils/CWEval/cweval/evaluate.py
"""
Pipeline utilities to parse tasks, compile (for compiled langs), run pytest,
and emit detailed JSON reports.

Usage examples:

# Run tests only and write per-generated res.json + 3 consolidated reports
python cweval/evaluate.py run_tests --eval_path evals/eval_LLMs__gpt-4o

# Full pipeline (parse -> compile -> tests)
python cweval/evaluate.py pipeline --eval_path evals/eval_LLMs__gpt-4o --docker False
"""

import datetime
import json
import math
import multiprocessing as mp
import os
import shutil
from typing import Dict, List, Tuple, Optional

import warnings
warnings.filterwarnings("ignore", category=UserWarning)

import fire
from natsort import natsorted
from p_tqdm import p_map

from cweval.commons import (
    BENCHMARK_DIR,
    COMPILE_DIR,
    LANGS,
    LANGS_COMPILE,
    compile_list,
    complete_code,
    get_code_from,
    pass_at_k,
    run_in_subprocess,
)
from cweval.run_tests import TestFileResult, run_tests, task_key_from_path
from cweval.sandbox import Container


# ---------- Helpers for reports ----------

def ensure_dir(path: str) -> None:
    os.makedirs(path, exist_ok=True)


def classify_status(functional: Optional[bool], secure: Optional[bool]) -> str:
    if functional is True and secure is True:
        return "all_passed"
    if (functional is True and secure is False) or (functional is False and secure is True):
        return "partial_passed"
    return "all_failed"


def per_file_entry_for_unittests_json(fr: TestFileResult) -> Dict:
    entry = {
        "task_key": task_key_from_path(fr.file),
        "status": classify_status(fr.functional, fr.secure),
        "total_tests": 0,
        "total_passed": 0,
        "total_failed": 0,
        "success_rate": 0.0,
        "setup": None,
        "testcases": None,
        "capability": {"total": 0, "passed": 0, "failed": 0},
        "safety": {"total": 0, "passed": 0, "failed": 0},
        "errors": [],
    }

    # totals
    total = sum(1 for t in fr.test_cases if t.run and t.marker in ("functionality", "security") and "_unsafe" not in t.name)
    passed = sum(1 for t in fr.test_cases if t.run and t.passed and t.marker in ("functionality", "security") and "_unsafe" not in t.name)
    failed = total - passed

    entry["total_tests"] = total
    entry["total_passed"] = passed
    entry["total_failed"] = failed
    entry["success_rate"] = (passed / total * 100.0) if total > 0 else 0.0

    # group counts by marker
    cap_total = sum(1 for t in fr.test_cases if t.run and t.marker == "functionality" and "_unsafe" not in t.name)
    cap_pass = sum(1 for t in fr.test_cases if t.run and t.passed and t.marker == "functionality" and "_unsafe" not in t.name)
    saf_total = sum(1 for t in fr.test_cases if t.run and t.marker == "security" and "_unsafe" not in t.name)
    saf_pass = sum(1 for t in fr.test_cases if t.run and t.passed and t.marker == "security" and "_unsafe" not in t.name)

    entry["capability"] = {"total": cap_total, "passed": cap_pass, "failed": cap_total - cap_pass}
    entry["safety"] = {"total": saf_total, "passed": saf_pass, "failed": saf_total - saf_pass}

    # record failures
    entry["errors"] = [
        {
            "test_name": t.name,
            "marker": t.marker,
            "stdout": t.stdout,
            "stderr": t.stderr,
            "longrepr": t.longrepr,
        }
        for t in fr.test_cases
        if t.run and not t.passed and "_unsafe" not in t.name
    ]

    return entry


def per_file_entry_for_code_analysis_json(fr: TestFileResult) -> Dict:
    """
    Best-effort version of the user's desired 'CWEval_code_analysis.json'.
    We don't have model code/think outputs here, so we emit placeholders
    and be explicit about what we *can* extract from the unit tests.
    """
    status = classify_status(fr.functional, fr.secure)
    # The harness doesn't provide ground-truth code or code_output, so we leave them None
    return {
        "task_key": task_key_from_path(fr.file),
        "test_result": status,
        "code_output_with_tuning": None,      # not available in this pipeline
        "think_output_with_tuning": None,     # not available in this pipeline
        "ground_truth_code": None,            # not available in this pipeline
        "capability_results": [
            1 if t.passed else 0
            for t in fr.test_cases
            if t.run and t.marker == "functionality" and "_unsafe" not in t.name
        ],
        "safety_results": [
            1 if t.passed else 0
            for t in fr.test_cases
            if t.run and t.marker == "security" and "_unsafe" not in t.name
        ],
        "capability_pass_rate": (
            sum(1 for t in fr.test_cases if t.run and t.passed and t.marker == "functionality" and "_unsafe" not in t.name)
            / max(1, sum(1 for t in fr.test_cases if t.run and t.marker == "functionality" and "_unsafe" not in t.name))
        ),
        "safety_pass_rate": (
            sum(1 for t in fr.test_cases if t.run and t.passed and t.marker == "security" and "_unsafe" not in t.name)
            / max(1, sum(1 for t in fr.test_cases if t.run and t.marker == "security" and "_unsafe" not in t.name))
        ),
        "runtime": sum(t.duration for t in fr.test_cases),
        "setup": None,         # unknown here
        "testcases": None,     # unknown here
        "errors": [
            {
                "test_name": t.name,
                "marker": t.marker,
                "stdout": t.stdout,
                "stderr": t.stderr,
                "longrepr": t.longrepr,
            }
            for t in fr.test_cases
            if t.run and not t.passed and "_unsafe" not in t.name
        ],
    }


def build_summary_report(file_results: List[TestFileResult]) -> Dict:
    # summary section
    total_entries = len(file_results)
    error_cases = 0  # not tracked separately here; keep 0
    evaluated_cases = total_entries
    evaluation_rate = 100.0 if total_entries else 0.0

    # per-test stats
    total_tests = 0
    total_passed = 0
    zero_tests_task_keys = []
    for fr in file_results:
        total, passed, failed = fr.total_counts()
        total_tests += total
        total_passed += passed
        if total == 0:
            zero_tests_task_keys.append(task_key_from_path(fr.file))

    overall_success_rate = (total_passed / total_tests * 100.0) if total_tests > 0 else 0.0

    # success breakdown
    all_passed = []
    partial = []
    all_failed = []
    for fr in file_results:
        status = classify_status(fr.functional, fr.secure)
        key = task_key_from_path(fr.file)
        if status == "all_passed":
            all_passed.append(key)
        elif status == "partial_passed":
            partial.append(key)
        else:
            all_failed.append(key)

    total_count = max(1, len(file_results))
    def pct(x): return (len(x) / total_count) * 100.0

    return {
        "summary": {
            "total_entries": total_entries,
            "error_cases": error_cases,
            "evaluated_cases": evaluated_cases,
            "evaluation_rate": evaluation_rate,
        },
        "test_statistics": {
            "total_tests_run": total_tests,
            "total_tests_passed": total_passed,
            "total_tests_failed": total_tests - total_passed,
            "overall_success_rate": overall_success_rate,
            "task_keys_with_0_tests_run": zero_tests_task_keys,
        },
        "success_breakdown": {
            "all_passed": {
                "count": len(all_passed),
                "percentage": pct(all_passed),
                "task_keys": all_passed,
            },
            "partial_passed": {
                "count": len(partial),
                "percentage": pct(partial),
                "task_keys": partial,
            },
            "all_failed": {
                "count": len(all_failed),
                "percentage": pct(all_failed),
                "task_keys": all_failed,
            },
        },
    }


# ---------- Original Evaler with additions for consolidated JSONs ----------

class Evaler:

    entrypoint_anchor = 'BEGIN ENTRYPOINT'
    docker_user = 'ubuntu'
    repo_path_in_docker = f'/home/{docker_user}/CWEval'

    def __init__(self, eval_path: str = '', num_proc: int = 8):
        if not eval_path:
            # find the latest one under './evals'
            evals_dir = 'evals'
            evals = natsorted(
                filter(
                    lambda x: os.path.isdir(os.path.join(evals_dir, x))
                    and x.startswith('eval_'),
                    os.listdir(evals_dir),
                )
            )
            eval_path = os.path.join(evals_dir, evals[-1])

        self.num_proc = num_proc
        self.eval_path = eval_path  # evals/eval_241110_014704
        self.generated_paths = []
        for d in natsorted(os.listdir(self.eval_path)):
            if d.startswith('generated_'):
                self.generated_paths.append(os.path.join(self.eval_path, d))

        self.raw_files: List[str] = []
        self.task_files: List[str] = []  # parsed from raw_files

        # add all *_raw.* files to raw_files
        for generated_path in self.generated_paths:
            for root, dirs, files in os.walk(generated_path):
                if '__pycache__' in root:
                    continue
                for file in natsorted(files):
                    if '_raw.' in file:
                        self.raw_files.append(os.path.join(root, file))

        print(f'{len(self.raw_files) = }', flush=True)

    def _parse_raw_file(self, raw_file_path: str) -> str:
        # raw_code + lines after BEGIN ENTRYPOINT in ref_task_file
        with open(raw_file_path, 'r') as f:
            raw_str = f.read()

        raw_code = get_code_from(raw_str, only_first=True)
        if not raw_code:
            raw_code = raw_str

        # get the entrypoint from the corresponding task file
        for generated_path in self.generated_paths:
            if raw_file_path.startswith(generated_path.rstrip('/') + '/'):
                break
        rel_raw_file_path = os.path.relpath(raw_file_path, generated_path)
        ref_task_file_path = os.path.join(
            BENCHMARK_DIR, rel_raw_file_path.replace('_raw.', '_task.')
        )
        with open(ref_task_file_path, 'r') as ref_task_file:
            ref_task_code = ref_task_file.read()

        # TODO hack for python cases
        if self.entrypoint_anchor not in ref_task_code:
            return raw_code

        entrypoint_src_line = [
            line
            for line in ref_task_code.splitlines()
            if self.entrypoint_anchor in line
        ][0]
        entrypoint_code = ref_task_code.split(entrypoint_src_line)[1].strip()

        tot_code = f'{raw_code}\n\n{entrypoint_src_line}\n{entrypoint_code}\n'

        lang = os.path.splitext(raw_file_path)[1][1:]
        tot_code = complete_code(tot_code, lang)

        return tot_code

    def _fill_task_files(self) -> None:
        if len(self.task_files) > 0:
            return
        for generated_path in self.generated_paths:
            for root, dirs, files in os.walk(generated_path):
                if '__pycache__' in root:
                    continue
                for file in natsorted(files):
                    if '_task.' in file:
                        self.task_files.append(os.path.join(root, file))

    def _copy_test_files(self) -> None:
        # copy test files from benchmark to generated for testing
        self._fill_task_files()
        for task_file in self.task_files:
            test_file = (
                os.path.splitext(task_file.replace('_task.', '_test.'))[0] + '.py'
            )
            # evals/.../generated_?/core/c/cwe_022_0_c_task.c -> evals/.../generated_?
            for generated_path in self.generated_paths:
                if task_file.startswith(generated_path.rstrip('/') + '/'):
                    break
            rel_task_file_path = os.path.relpath(task_file, generated_path)
            ref_test_file_path = os.path.join(
                BENCHMARK_DIR,
                os.path.splitext(rel_task_file_path.replace('_task.', '_test.'))[0]
                + '.py',
            )
            shutil.copy(ref_test_file_path, test_file)

    def _merge_results(self) -> None:
        # merge the results from res.json files (legacy)
        all_res: Dict[str, Dict[str, List[bool]]] = {}
        for generated_path in self.generated_paths:
            res_json_path = os.path.join(generated_path, 'res.json')
            if not os.path.exists(res_json_path):
                continue
            with open(res_json_path, 'r') as f:
                res = json.load(f)
            for test_path, test_res in res.items():
                generated_name = os.path.basename(generated_path)
                path_key = test_path.replace(generated_name, f'generated_X')
                all_res.setdefault(path_key, {"functional": [], "secure": [], "func_secure": []})
                all_res[path_key]["functional"].append(test_res["functional"])
                all_res[path_key]["secure"].append(test_res["secure"])
                all_res[path_key]["func_secure"].append(test_res["functional"] and test_res["secure"])

        with open(os.path.join(self.eval_path, 'res_all.json'), 'w') as f:
            json.dump(all_res, f, indent=2)

    def _filename_to_lang(self, path: str) -> str:
        filename = os.path.splitext(os.path.basename(path))[0]
        lang = filename.split('_')[-2]
        if lang.isdigit():
            return 'py'
        return lang

    def report_pass_at_k(
        self, k: int = 1, lang: str = '', mode: str = 'auto'
    ) -> Tuple[float, float, float] | None:
        if mode == 'auto':
            for _lang in [f'core/{_l}/' for _l in LANGS] + [f'lang/c'] + ['']:
                for _k in [1, 10, 50]:
                    self.report_pass_at_k(_k, _lang, mode='')
            return

        all_res_json_path = os.path.join(self.eval_path, 'res_all.json')
        if not os.path.exists(all_res_json_path):
            print(f"No res_all.json found at {all_res_json_path}")
            return
        with open(all_res_json_path, 'r') as f:
            all_res = json.load(f)

        num_samples = len(list(all_res.values())[0]['functional'])
        if num_samples < k:
            return

        # filter by lang
        if lang:
            all_res = {path: v for path, v in all_res.items() if lang in path}

        num_paths = len(all_res)
        if num_paths == 0:
            return

        functional_patks: List[float] = []
        secure_patks: List[float] = []
        func_secure_patks: List[float] = []
        for path, res in all_res.items():
            functional_patk = pass_at_k(len(res['functional']), sum(res['functional']), k)
            secure_patk = pass_at_k(len(res['secure']), sum(res['secure']), k)
            func_secure_patk = pass_at_k(len(res['func_secure']), sum(res['func_secure']), k)
            functional_patks.append(functional_patk)
            secure_patks.append(secure_patk)
            func_secure_patks.append(func_secure_patk)

        functional_rate = sum(functional_patks) / num_paths * 100
        secure_rate = sum(secure_patks) / num_paths * 100
        func_secure_rate = sum(func_secure_patks) / num_paths * 100

        print(f'=' * 16)
        print(f'pass@{k}\t{lang or "all"}')
        print(f'func@{k}\t{functional_rate:.2f}')
        print(f'func-sec@{k}\t{func_secure_rate:.2f}')
        print(f'=' * 16)

        return functional_rate, secure_rate, func_secure_rate

    def _parse_raw_write_task(self, raw_file: str) -> None:
        task_code = self._parse_raw_file(raw_file)
        task_file = raw_file.replace('_raw.', '_task.')
        with open(task_file, 'w') as f:
            f.write(task_code)

    def parse_generated(self) -> None:
        if self.num_proc == 1:
            for raw_file in natsorted(self.raw_files):
                task_code = self._parse_raw_file(raw_file)
                task_file = raw_file.replace('_raw.', '_task.')
                self.task_files.append(task_file)
                with open(task_file, 'w') as f:
                    f.write(task_code)
        else:
            print(f'Parsing {len(self.raw_files)} files with {self.num_proc * 2} processes', flush=True)
            p_map(self._parse_raw_write_task, self.raw_files, num_cpus=self.num_proc * 2)

    def compile_parsed(self) -> None:
        self._fill_task_files()
        to_compile_files = [
            task_file
            for task_file in self.task_files
            if any(task_file.endswith(f'.{lang}') for lang in LANGS_COMPILE)
        ]
        compiled_files = [
            os.path.join(os.path.dirname(task_file), COMPILE_DIR, os.path.splitext(os.path.basename(task_file))[0])
            for task_file in to_compile_files
        ]
        compile_list(to_compile_files, compiled_files, check=False, num_proc=self.num_proc)

    def run_tests(self) -> None:
        self._copy_test_files()
        all_gen_results: List[List[TestFileResult]] = []
        if self.num_proc == 1:
            for generated_path in self.generated_paths:
                file_res_list = run_in_subprocess(run_tests, generated_path)
                all_gen_results.append(file_res_list)
        else:
            mp.set_start_method('spawn', force=True)
            all_gen_results = []
            for i in range(math.ceil(len(self.generated_paths) / self.num_proc)):
                generated_paths_i = self.generated_paths[i * self.num_proc : (i + 1) * self.num_proc]
                assert len(generated_paths_i) <= self.num_proc
                with mp.Pool(self.num_proc, maxtasksperchild=1) as pool:
                    gen_results_i = pool.map(run_tests, generated_paths_i, chunksize=1)
                all_gen_results.extend(gen_results_i)
                print(f'Finished {i = } th batch', flush=True)

        print(f'Finished running tests in {self.eval_path = }', flush=True)

        # legacy per-generated res.json and consolidated JSONs in results/CWEval/LLMs/gpt-4o
        for file_res_list, generated_path in zip(all_gen_results, self.generated_paths):
            all_res = {
                file_res.file: {
                    'functional': file_res.functional,
                    'secure': file_res.secure,
                }
                for file_res in file_res_list
            }
            res_json_path = os.path.join(generated_path, 'res.json')
            with open(res_json_path, 'w') as f:
                json.dump(all_res, f, indent=4)

        # New consolidated reports
        flat_results: List[TestFileResult] = [fr for sub in all_gen_results for fr in sub]
        # out_dir = os.path.join("..", "..", "results", "CWEval", "LLMs", "gpt-4o")
        eval_dir_name = os.path.basename(self.eval_path)  # e.g., eval_deepseek-coder-7b__CoT-SFT_RLVR
        model_name, variant_name = tuple(eval_dir_name.replace("eval_", "", 1).split("__"))  # deepseek-coder-7b__CoT-SFT_RLVR -> (deepseek-coder-7b, CoT-SFT_RLVR)
        # out_dir = os.path.join("..", "..", "results", "CWEval", model_name, variant_name) # to be used when running outside docker
        
        # Use absolute path to mounted results directory
        out_dir = os.path.join("/home/ubuntu/results", "CWEval", model_name, variant_name) # to be used when running inside docker
        ensure_dir(out_dir)

        # 1) CWEval_unittests_results.json  (per-file structured results)
        unit_json = [per_file_entry_for_unittests_json(fr) for fr in flat_results]
        with open(os.path.join(out_dir, "CWEval_unittests_results.json"), "w") as f:
            json.dump(unit_json, f, indent=2)

        # 2) CWEval_unittests_results_analysis_report.json (summary)
        analysis = build_summary_report(flat_results)
        with open(os.path.join(out_dir, "CWEval_unittests_results_analysis_report.json"), "w") as f:
            json.dump(analysis, f, indent=2)

        # 3) CWEval_code_analysis.json (best-effort mapping)
        code_json = [per_file_entry_for_code_analysis_json(fr) for fr in flat_results]
        with open(os.path.join(out_dir, "CWEval_code_analysis.json"), "w") as f:
            json.dump(code_json, f, indent=2)

    def run_tests_in_docker(self, prepare: bool = True) -> None:
        if prepare:
            self.parse_generated()
            self.compile_parsed()
        print(f'Run docker', flush=True)
        timestamp = datetime.datetime.now().strftime('%y%m%d_%H%M%S')
        container = Container(
            image='co1lin/cweval',
            name=f'cweval_{timestamp}',
            user=self.docker_user,
        )
        evals_path_in_docker = os.path.join(self.repo_path_in_docker, 'evals')
        eval_path_in_docker = os.path.join(evals_path_in_docker, os.path.basename(self.eval_path))
        container.exec_cmd(
            f'''bash -c "
mkdir -p {evals_path_in_docker};
rm -rf {eval_path_in_docker}
"'''
        )
        container.copy_to(self.eval_path, eval_path_in_docker)
        log_path_in_docker = os.path.join(eval_path_in_docker, 'run_tests.log')
        cmd = f'''set -e;
source /home/{self.docker_user}/miniforge3/bin/activate;
cd {self.repo_path_in_docker};
source .env;
python cweval/evaluate.py run_tests --eval_path {eval_path_in_docker} --num_proc {self.num_proc} 2>&1 | tee {log_path_in_docker};
'''
        cmd = f'bash -c "{cmd}"'
        exit_code, stdout, stderr = container.exec_cmd(cmd)
        assert exit_code == 0, f'{exit_code = }\nstdout:\n{stdout}\n\nstderr:\n{stderr}'
        log_path = os.path.join(self.eval_path, 'run_tests.log')
        container.copy_from(log_path_in_docker, log_path)
        for generated_path in self.generated_paths:
            res_json_path = os.path.join(generated_path, 'res.json')
            res_json_path_in_docker = os.path.join(
                eval_path_in_docker, os.path.relpath(res_json_path, self.eval_path)
            )
            container.copy_from(res_json_path_in_docker, res_json_path)

    def pipeline(self, docker: bool = True) -> None:
        self.parse_generated()
        self.compile_parsed()
        if isinstance(docker, str):
            docker = docker.lower() == 'true'
        assert isinstance(docker, bool), f'{docker = }'
        if docker:
            self.run_tests_in_docker(prepare=False)
        else:
            self.run_tests()
        self._merge_results()
        self.report_pass_at_k(mode='auto')


if __name__ == '__main__':
    fire.Fire(Evaler)


# """
# Expected directory structure:

# evals
# ├── eval_241110_014704
# │   ├── generated_0
# │   │   ├── core
# │   │   │   ├── c
# │   │   │   │   ├── compiled
# │   │   │   │   │   └── cwe_022_0_c_task
# │   │   │   │   ├── cwe_022_0_c_raw.c
# │   │   │   │   ├── cwe_022_0_c_task.c    <--- Parse from _raw
# │   │   │   │   ├── cwe_022_0_c_test.py    <--- Copy from benchmark
# │   │   │   └── py
# │   │   │       ├── cwe_020_0_raw.py
# │   │   │       ├── cwe_020_0_task.py
# │   │   │       ├── cwe_020_0_test.py
# │   │   └── lang
# │   │   └── res.json    <--- Run tests to get
# │   └── generated_1
# └── pytest.ini
# """

# import datetime
# import json
# import math
# import multiprocessing as mp
# import os
# import shutil
# from typing import Dict, List, Tuple

# import fire
# from natsort import natsorted
# from p_tqdm import p_map

# from cweval.commons import (
#     BENCHMARK_DIR,
#     COMPILE_DIR,
#     LANGS,
#     LANGS_COMPILE,
#     compile_list,
#     complete_code,
#     get_code_from,
#     pass_at_k,
#     run_in_subprocess,
# )
# from cweval.run_tests import TestFileResult, run_tests
# from cweval.sandbox import Container


# class Evaler:

#     entrypoint_anchor = 'BEGIN ENTRYPOINT'
#     docker_user = 'ubuntu'
#     repo_path_in_docker = f'/home/{docker_user}/CWEval'

#     def __init__(self, eval_path: str = '', num_proc: int = 8):
#         if not eval_path:
#             # find the latest one under './evals'
#             evals_dir = 'evals'
#             evals = natsorted(
#                 filter(
#                     lambda x: os.path.isdir(os.path.join(evals_dir, x))
#                     and x.startswith('eval_'),
#                     os.listdir(evals_dir),
#                 )
#             )
#             eval_path = os.path.join(evals_dir, evals[-1])

#         self.num_proc = num_proc
#         self.eval_path = eval_path  # evals/eval_241110_014704
#         self.generated_paths = []
#         for d in natsorted(os.listdir(self.eval_path)):
#             if d.startswith('generated_'):
#                 self.generated_paths.append(os.path.join(self.eval_path, d))

#         self.raw_files: List[str] = []
#         self.task_files: List[str] = []  # parsed from raw_files

#         # add all *_raw.* files to raw_files
#         for generated_path in self.generated_paths:
#             for root, dirs, files in os.walk(generated_path):
#                 if '__pycache__' in root:
#                     continue
#                 for file in natsorted(files):
#                     if '_raw.' in file:
#                         self.raw_files.append(os.path.join(root, file))

#         print(f'{len(self.raw_files) = }', flush=True)

#     def _parse_raw_file(self, raw_file_path: str) -> str:
#         # raw_code + lines after BEGIN ENTRYPOINT in ref_task_file
#         # python cweval/evaluate.py _parse_raw_file --eval_path evals/eval_241110_014704
#         with open(raw_file_path, 'r') as f:
#             raw_str = f.read()

#         raw_code = get_code_from(raw_str, only_first=True)
#         if not raw_code:
#             raw_code = raw_str

#         # get the entrypoint from the corresponding task file
#         for generated_path in self.generated_paths:
#             if raw_file_path.startswith(generated_path.rstrip('/') + '/'):
#                 break
#         rel_raw_file_path = os.path.relpath(raw_file_path, generated_path)
#         ref_task_file_path = os.path.join(
#             BENCHMARK_DIR, rel_raw_file_path.replace('_raw.', '_task.')
#         )
#         with open(ref_task_file_path, 'r') as ref_task_file:
#             ref_task_code = ref_task_file.read()

#         # TODO hack for python cases
#         if self.entrypoint_anchor not in ref_task_code:
#             return raw_code

#         entrypoint_src_line = [
#             line
#             for line in ref_task_code.splitlines()
#             if self.entrypoint_anchor in line
#         ][0]
#         entrypoint_code = ref_task_code.split(entrypoint_src_line)[1].strip()

#         tot_code = f'{raw_code}\n\n{entrypoint_src_line}\n{entrypoint_code}\n'

#         lang = os.path.splitext(raw_file_path)[1][1:]
#         tot_code = complete_code(tot_code, lang)

#         return tot_code

#     def _fill_task_files(self) -> None:
#         # fill the task_files with the task files
#         if len(self.task_files) > 0:
#             return
#         for generated_path in self.generated_paths:
#             for root, dirs, files in os.walk(generated_path):
#                 if '__pycache__' in root:
#                     continue
#                 for file in natsorted(files):
#                     if '_task.' in file:
#                         self.task_files.append(os.path.join(root, file))

#     def _copy_test_files(self) -> None:
#         # copy test files from benchmark to generated for testing
#         self._fill_task_files()
#         for task_file in self.task_files:
#             test_file = (
#                 os.path.splitext(task_file.replace('_task.', '_test.'))[0] + '.py'
#             )
#             # evals/eval_241110_014704/generated_?/core/c/cwe_022_0_c_task.c -> evals/eval_241110_014704/generated_?
#             for generated_path in self.generated_paths:
#                 if task_file.startswith(generated_path.rstrip('/') + '/'):
#                     break
#             rel_task_file_path = os.path.relpath(task_file, generated_path)
#             ref_test_file_path = os.path.join(
#                 BENCHMARK_DIR,
#                 os.path.splitext(rel_task_file_path.replace('_task.', '_test.'))[0]
#                 + '.py',
#             )
#             # print(f'{ref_test_file_path} ==>> {test_file}')
#             shutil.copy(ref_test_file_path, test_file)

#     def _merge_results(self) -> None:
#         # python cweval/evaluate.py _merge_results --eval_path evals/eval_241110_014704
#         # merge the results from res.json files
#         all_res: Dict[str, Dict[str, List[bool]]] = {}
#         for generated_path in self.generated_paths:
#             res_json_path = os.path.join(generated_path, 'res.json')
#             with open(res_json_path, 'r') as f:
#                 res = json.load(f)
#             for test_path, test_res in res.items():
#                 # evals/eval_241110_014704/generated_?/core/c/cwe_022_0_c_test.py -> evals/eval_241110_014704/generated_X/core/c/cwe_022_0_c_test.py
#                 generated_name = os.path.basename(generated_path)
#                 path_key = test_path.replace(generated_name, f'generated_X')
#                 all_res[path_key] = all_res.get(
#                     path_key,
#                     {
#                         'functional': [],
#                         'secure': [],
#                         'func_secure': [],
#                     },
#                 )
#                 all_res[path_key]['functional'].append(test_res['functional'])
#                 all_res[path_key]['secure'].append(test_res['secure'])
#                 all_res[path_key]['func_secure'].append(
#                     test_res['functional'] and test_res['secure']
#                 )

#         with open(os.path.join(self.eval_path, 'res_all.json'), 'w') as f:
#             json.dump(all_res, f, indent=2)

#     def _filename_to_lang(self, path: str) -> str:
#         # path: evals/eval_241110_014704/generated_X/<...>/cwe_022_0_c_test.py -> c
#         # evals/eval_241110_014704/generated_X/<...>/cwe_022_0_test.py -> py
#         filename = os.path.splitext(os.path.basename(path))[0]
#         lang = filename.split('_')[-2]
#         if lang.isdigit():
#             return 'py'
#         return lang

#     def report_pass_at_k(
#         self, k: int = 1, lang: str = '', mode: str = 'auto'
#     ) -> Tuple[float, float, float] | None:
#         if mode == 'auto':
#             for _lang in [f'core/{_l}/' for _l in LANGS] + [f'lang/c'] + ['']:
#                 for _k in [
#                     1,
#                     10,
#                     50,
#                 ]:
#                     self.report_pass_at_k(_k, _lang, mode='')
#             return

#         all_res_json_path = os.path.join(self.eval_path, 'res_all.json')
#         # all_res_json_path = os.path.join(self.eval_path, 'res.json')
#         with open(all_res_json_path, 'r') as f:
#             all_res = json.load(f)

#         num_samples = len(list(all_res.values())[0]['functional'])
#         if num_samples < k:
#             return

#         # filter by lang
#         if lang:
#             all_res = {path: v for path, v in all_res.items() if lang in path}

#         num_paths = len(all_res)
#         if num_paths == 0:
#             # print(f'No case found for {lang = }')
#             return

#         functional_patks: List[float] = []
#         secure_patks: List[float] = []
#         func_secure_patks: List[float] = []
#         # secure_when_func_patks: List[float] = []
#         for path, res in all_res.items():
#             functional_patk = pass_at_k(
#                 len(res['functional']),
#                 sum(res['functional']),
#                 k,
#             )
#             # assert not any(not functional and secure for functional, secure in zip(res['functional'], res['secure'])), f'{path = } has a test case that is not functional but secure, which is impossible'
#             secure_patk = pass_at_k(
#                 len(res['secure']),
#                 sum(res['secure']),
#                 k,
#             )
#             func_secure_patk = pass_at_k(
#                 len(res['func_secure']),
#                 sum(res['func_secure']),
#                 k,
#             )

#             # first_50_func_is_secure = []
#             # for i, (functional, secure) in enumerate(zip(res['functional'], res['secure'])):
#             #     if functional:
#             #         first_50_func_is_secure.append(secure)
#             #     if len(first_50_func_is_secure) == 50:
#             #         break
#             # # assert len(first_50_func_is_secure) == 50, f'{len(first_50_func_is_secure) = }'
#             # if len(first_50_func_is_secure) == 50:
#             #     secure_when_func_patk = pass_at_k(
#             #         50,
#             #         sum(first_50_func_is_secure),
#             #         k,
#             #     )
#             #     secure_when_func_patks.append(secure_when_func_patk)

#             functional_patks.append(functional_patk)
#             secure_patks.append(secure_patk)
#             func_secure_patks.append(func_secure_patk)

#         functional_rate = sum(functional_patks) / num_paths * 100
#         secure_rate = sum(secure_patks) / num_paths * 100
#         func_secure_rate = sum(func_secure_patks) / num_paths * 100
#         # secure_when_func_rate = sum(secure_when_func_patks) / num_paths * 100

#         print(f'=' * 16)
#         print(f'pass@{k}\t{lang or "all"}')
#         print(f'func@{k}\t{functional_rate:.2f}')
#         # print(f'secure@{k}\t{secure_rate:.2f}')
#         print(f'func-sec@{k}\t{func_secure_rate:.2f}')
#         # print(f'secure_when_functional@{k}\t{secure_when_func_rate:.2f}')
#         print(f'=' * 16)

#         return functional_rate, secure_rate, func_secure_rate

#     def _parse_raw_write_task(self, raw_file: str) -> None:
#         task_code = self._parse_raw_file(raw_file)
#         task_file = raw_file.replace('_raw.', '_task.')
#         with open(task_file, 'w') as f:
#             f.write(task_code)

#     def parse_generated(self) -> None:
#         # python cweval/evaluate.py parse_generated --eval_path evals/eval_241110_014704
#         # parse the raw_files to get the task_files
#         if self.num_proc == 1:
#             for raw_file in natsorted(self.raw_files):
#                 task_code = self._parse_raw_file(raw_file)
#                 task_file = raw_file.replace('_raw.', '_task.')
#                 self.task_files.append(task_file)
#                 with open(task_file, 'w') as f:
#                     f.write(task_code)
#         else:
#             print(
#                 f'Parsing {len(self.raw_files)} files with {self.num_proc * 2} processes',
#                 flush=True,
#             )
#             p_map(
#                 self._parse_raw_write_task, self.raw_files, num_cpus=self.num_proc * 2
#             )

#     def compile_parsed(self) -> None:
#         # python cweval/evaluate.py compile_parsed --eval_path evals/eval_241110_014704
#         self._fill_task_files()
#         # compile C
#         to_compile_files = [
#             task_file
#             for task_file in self.task_files
#             if any(task_file.endswith(f'.{lang}') for lang in LANGS_COMPILE)
#         ]
#         # {c_files_dir}/{COMPILE_DIR}/{name_of_c_file}
#         compiled_files = [
#             os.path.join(
#                 os.path.dirname(task_file),
#                 COMPILE_DIR,
#                 os.path.splitext(os.path.basename(task_file))[0],
#             )
#             for task_file in to_compile_files
#         ]
#         compile_list(
#             to_compile_files, compiled_files, check=False, num_proc=self.num_proc
#         )

#     def run_tests(self) -> None:
#         # python cweval/evaluate.py run_tests --eval_path evals/eval_241110_014704
#         self._copy_test_files()
#         all_gen_results = []
#         if self.num_proc == 1:
#             for generated_path in self.generated_paths:
#                 # file_res_list = run_tests(generated_path)
#                 file_res_list = run_in_subprocess(run_tests, generated_path)
#                 all_gen_results.append(file_res_list)
#         else:
#             mp.set_start_method('spawn', force=True)
#             all_gen_results: List[TestFileResult] = []
#             # fix mysterious hanging issue
#             for i in range(math.ceil(len(self.generated_paths) / self.num_proc)):
#                 generated_paths_i = self.generated_paths[
#                     i * self.num_proc : (i + 1) * self.num_proc
#                 ]
#                 assert len(generated_paths_i) <= self.num_proc
#                 with mp.Pool(self.num_proc, maxtasksperchild=1) as pool:
#                     gen_results_i = pool.map(run_tests, generated_paths_i, chunksize=1)
#                 all_gen_results.extend(gen_results_i)
#                 print(f'Finished {i = } th batch', flush=True)

#             # with mp.Pool(self.num_proc, maxtasksperchild=1) as pool:
#             #     all_gen_results = pool.map(run_tests, self.generated_paths, chunksize=1)

#         print(f'Finished running tests in {self.eval_path = }', flush=True)

#         for file_res_list, generated_path in zip(all_gen_results, self.generated_paths):
#             all_res = {
#                 file_res.file: {
#                     'functional': file_res.functional,
#                     'secure': file_res.secure,
#                 }
#                 for file_res in file_res_list
#             }
#             res_json_path = os.path.join(generated_path, 'res.json')
#             with open(res_json_path, 'w') as f:
#                 json.dump(all_res, f, indent=4)

#     def run_tests_in_docker(self, prepare: bool = True) -> None:
#         if prepare:
#             self.parse_generated()
#             self.compile_parsed()
#         print(f'Run docker', flush=True)
#         timestamp = datetime.datetime.now().strftime('%y%m%d_%H%M%S')
#         container = Container(
#             image='co1lin/cweval',
#             name=f'cweval_{timestamp}',
#             user=self.docker_user,
#         )
#         # prepare the files in the container
#         evals_path_in_docker = os.path.join(
#             self.repo_path_in_docker, 'evals'
#         )  # /home/ubuntu/CWEval/evals
#         eval_path_in_docker = os.path.join(
#             evals_path_in_docker, os.path.basename(self.eval_path)
#         )  # /home/ubuntu/CWEval/evals/eval_241110_014704
#         container.exec_cmd(
#             f'''bash -c "
# mkdir -p {evals_path_in_docker};
# rm -rf {eval_path_in_docker}
# "'''
#         )
#         container.copy_to(self.eval_path, eval_path_in_docker)
#         log_path_in_docker = os.path.join(
#             eval_path_in_docker, 'run_tests.log'
#         )  # /home/ubuntu/CWEval/evals/eval_241110_014704/run_tests.log
#         # run the tests
#         cmd = f'''set -e;
# source /home/{self.docker_user}/miniforge3/bin/activate;
# cd {self.repo_path_in_docker};
# source .env;
# python cweval/evaluate.py run_tests --eval_path {eval_path_in_docker} --num_proc {self.num_proc} 2>&1 | tee {log_path_in_docker};
# '''
#         cmd = f'bash -c "{cmd}"'
#         exit_code, stdout, stderr = container.exec_cmd(cmd)
#         assert exit_code == 0, f'{exit_code = }\nstdout:\n{stdout}\n\nstderr:\n{stderr}'
#         # copy the log file and results
#         log_path = os.path.join(
#             self.eval_path, 'run_tests.log'
#         )  # evals/eval_241110_014704/run_tests.log
#         container.copy_from(log_path_in_docker, log_path)
#         for generated_path in self.generated_paths:
#             res_json_path = os.path.join(
#                 generated_path, 'res.json'
#             )  # evals/eval_241110_014704/generated_X/res.json
#             res_json_path_in_docker = os.path.join(
#                 eval_path_in_docker, os.path.relpath(res_json_path, self.eval_path)
#             )  # /home/ubuntu/CWEval/evals/eval_241110_014704/generated_X/res.json
#             container.copy_from(res_json_path_in_docker, res_json_path)

#     def pipeline(self, docker: bool = True) -> None:
#         self.parse_generated()
#         self.compile_parsed()
#         if isinstance(docker, str):
#             docker = docker.lower() == 'true'
#         assert isinstance(docker, bool), f'{docker = }'
#         if docker:
#             self.run_tests_in_docker(prepare=False)
#         else:
#             self.run_tests()
#         self._merge_results()
#         self.report_pass_at_k(mode='auto')


# if __name__ == '__main__':
#     fire.Fire(Evaler)

# # UNCHANGED utils/CWEval/cweval/evaluate.py
# """
# Expected directory structure:

# evals
# ├── eval_241110_014704
# │   ├── generated_0
# │   │   ├── core
# │   │   │   ├── c
# │   │   │   │   ├── compiled
# │   │   │   │   │   └── cwe_022_0_c_task
# │   │   │   │   ├── cwe_022_0_c_raw.c
# │   │   │   │   ├── cwe_022_0_c_task.c    <--- Parse from _raw
# │   │   │   │   ├── cwe_022_0_c_test.py    <--- Copy from benchmark
# │   │   │   └── py
# │   │   │       ├── cwe_020_0_raw.py
# │   │   │       ├── cwe_020_0_task.py
# │   │   │       ├── cwe_020_0_test.py
# │   │   └── lang
# │   │   └── res.json    <--- Run tests to get
# │   └── generated_1
# └── pytest.ini
# """

# import datetime
# import json
# import math
# import multiprocessing as mp
# import os
# import shutil
# from typing import Dict, List, Tuple

# import fire
# from natsort import natsorted
# from p_tqdm import p_map

# from cweval.commons import (
#     BENCHMARK_DIR,
#     COMPILE_DIR,
#     LANGS,
#     LANGS_COMPILE,
#     compile_list,
#     complete_code,
#     get_code_from,
#     pass_at_k,
#     run_in_subprocess,
# )
# from cweval.run_tests import TestFileResult, run_tests
# from cweval.sandbox import Container


# class Evaler:

#     entrypoint_anchor = 'BEGIN ENTRYPOINT'
#     docker_user = 'ubuntu'
#     repo_path_in_docker = f'/home/{docker_user}/CWEval'

#     def __init__(self, eval_path: str = '', num_proc: int = 8):
#         if not eval_path:
#             # find the latest one under './evals'
#             evals_dir = 'evals'
#             evals = natsorted(
#                 filter(
#                     lambda x: os.path.isdir(os.path.join(evals_dir, x))
#                     and x.startswith('eval_'),
#                     os.listdir(evals_dir),
#                 )
#             )
#             eval_path = os.path.join(evals_dir, evals[-1])

#         self.num_proc = num_proc
#         self.eval_path = eval_path  # evals/eval_241110_014704
#         self.generated_paths = []
#         for d in natsorted(os.listdir(self.eval_path)):
#             if d.startswith('generated_'):
#                 self.generated_paths.append(os.path.join(self.eval_path, d))

#         self.raw_files: List[str] = []
#         self.task_files: List[str] = []  # parsed from raw_files

#         # add all *_raw.* files to raw_files
#         for generated_path in self.generated_paths:
#             for root, dirs, files in os.walk(generated_path):
#                 if '__pycache__' in root:
#                     continue
#                 for file in natsorted(files):
#                     if '_raw.' in file:
#                         self.raw_files.append(os.path.join(root, file))

#         print(f'{len(self.raw_files) = }', flush=True)

#     def _parse_raw_file(self, raw_file_path: str) -> str:
#         # raw_code + lines after BEGIN ENTRYPOINT in ref_task_file
#         # python cweval/evaluate.py _parse_raw_file --eval_path evals/eval_241110_014704
#         with open(raw_file_path, 'r') as f:
#             raw_str = f.read()

#         raw_code = get_code_from(raw_str, only_first=True)
#         if not raw_code:
#             raw_code = raw_str

#         # get the entrypoint from the corresponding task file
#         for generated_path in self.generated_paths:
#             if raw_file_path.startswith(generated_path.rstrip('/') + '/'):
#                 break
#         rel_raw_file_path = os.path.relpath(raw_file_path, generated_path)
#         ref_task_file_path = os.path.join(
#             BENCHMARK_DIR, rel_raw_file_path.replace('_raw.', '_task.')
#         )
#         with open(ref_task_file_path, 'r') as ref_task_file:
#             ref_task_code = ref_task_file.read()

#         # TODO hack for python cases
#         if self.entrypoint_anchor not in ref_task_code:
#             return raw_code

#         entrypoint_src_line = [
#             line
#             for line in ref_task_code.splitlines()
#             if self.entrypoint_anchor in line
#         ][0]
#         entrypoint_code = ref_task_code.split(entrypoint_src_line)[1].strip()

#         tot_code = f'{raw_code}\n\n{entrypoint_src_line}\n{entrypoint_code}\n'

#         lang = os.path.splitext(raw_file_path)[1][1:]
#         tot_code = complete_code(tot_code, lang)

#         return tot_code

#     def _fill_task_files(self) -> None:
#         # fill the task_files with the task files
#         if len(self.task_files) > 0:
#             return
#         for generated_path in self.generated_paths:
#             for root, dirs, files in os.walk(generated_path):
#                 if '__pycache__' in root:
#                     continue
#                 for file in natsorted(files):
#                     if '_task.' in file:
#                         self.task_files.append(os.path.join(root, file))

#     def _copy_test_files(self) -> None:
#         # copy test files from benchmark to generated for testing
#         self._fill_task_files()
#         for task_file in self.task_files:
#             test_file = (
#                 os.path.splitext(task_file.replace('_task.', '_test.'))[0] + '.py'
#             )
#             # evals/eval_241110_014704/generated_?/core/c/cwe_022_0_c_task.c -> evals/eval_241110_014704/generated_?
#             for generated_path in self.generated_paths:
#                 if task_file.startswith(generated_path.rstrip('/') + '/'):
#                     break
#             rel_task_file_path = os.path.relpath(task_file, generated_path)
#             ref_test_file_path = os.path.join(
#                 BENCHMARK_DIR,
#                 os.path.splitext(rel_task_file_path.replace('_task.', '_test.'))[0]
#                 + '.py',
#             )
#             # print(f'{ref_test_file_path} ==>> {test_file}')
#             shutil.copy(ref_test_file_path, test_file)

#     def _merge_results(self) -> None:
#         # python cweval/evaluate.py _merge_results --eval_path evals/eval_241110_014704
#         # merge the results from res.json files
#         all_res: Dict[str, Dict[str, List[bool]]] = {}
#         for generated_path in self.generated_paths:
#             res_json_path = os.path.join(generated_path, 'res.json')
#             with open(res_json_path, 'r') as f:
#                 res = json.load(f)
#             for test_path, test_res in res.items():
#                 # evals/eval_241110_014704/generated_?/core/c/cwe_022_0_c_test.py -> evals/eval_241110_014704/generated_X/core/c/cwe_022_0_c_test.py
#                 generated_name = os.path.basename(generated_path)
#                 path_key = test_path.replace(generated_name, f'generated_X')
#                 all_res[path_key] = all_res.get(
#                     path_key,
#                     {
#                         'functional': [],
#                         'secure': [],
#                         'func_secure': [],
#                     },
#                 )
#                 all_res[path_key]['functional'].append(test_res['functional'])
#                 all_res[path_key]['secure'].append(test_res['secure'])
#                 all_res[path_key]['func_secure'].append(
#                     test_res['functional'] and test_res['secure']
#                 )

#         with open(os.path.join(self.eval_path, 'res_all.json'), 'w') as f:
#             json.dump(all_res, f, indent=2)

#     def _filename_to_lang(self, path: str) -> str:
#         # path: evals/eval_241110_014704/generated_X/<...>/cwe_022_0_c_test.py -> c
#         # evals/eval_241110_014704/generated_X/<...>/cwe_022_0_test.py -> py
#         filename = os.path.splitext(os.path.basename(path))[0]
#         lang = filename.split('_')[-2]
#         if lang.isdigit():
#             return 'py'
#         return lang

#     def report_pass_at_k(
#         self, k: int = 1, lang: str = '', mode: str = 'auto'
#     ) -> Tuple[float, float, float] | None:
#         if mode == 'auto':
#             for _lang in [f'core/{_l}/' for _l in LANGS] + [f'lang/c'] + ['']:
#                 for _k in [
#                     1,
#                     10,
#                     50,
#                 ]:
#                     self.report_pass_at_k(_k, _lang, mode='')
#             return

#         all_res_json_path = os.path.join(self.eval_path, 'res_all.json')
#         with open(all_res_json_path, 'r') as f:
#             all_res = json.load(f)

#         num_samples = len(list(all_res.values())[0]['functional'])
#         if num_samples < k:
#             return

#         # filter by lang
#         if lang:
#             all_res = {path: v for path, v in all_res.items() if lang in path}

#         num_paths = len(all_res)
#         if num_paths == 0:
#             # print(f'No case found for {lang = }')
#             return

#         functional_patks: List[float] = []
#         secure_patks: List[float] = []
#         func_secure_patks: List[float] = []
#         # secure_when_func_patks: List[float] = []
#         for path, res in all_res.items():
#             functional_patk = pass_at_k(
#                 len(res['functional']),
#                 sum(res['functional']),
#                 k,
#             )
#             # assert not any(not functional and secure for functional, secure in zip(res['functional'], res['secure'])), f'{path = } has a test case that is not functional but secure, which is impossible'
#             secure_patk = pass_at_k(
#                 len(res['secure']),
#                 sum(res['secure']),
#                 k,
#             )
#             func_secure_patk = pass_at_k(
#                 len(res['func_secure']),
#                 sum(res['func_secure']),
#                 k,
#             )

#             # first_50_func_is_secure = []
#             # for i, (functional, secure) in enumerate(zip(res['functional'], res['secure'])):
#             #     if functional:
#             #         first_50_func_is_secure.append(secure)
#             #     if len(first_50_func_is_secure) == 50:
#             #         break
#             # # assert len(first_50_func_is_secure) == 50, f'{len(first_50_func_is_secure) = }'
#             # if len(first_50_func_is_secure) == 50:
#             #     secure_when_func_patk = pass_at_k(
#             #         50,
#             #         sum(first_50_func_is_secure),
#             #         k,
#             #     )
#             #     secure_when_func_patks.append(secure_when_func_patk)

#             functional_patks.append(functional_patk)
#             secure_patks.append(secure_patk)
#             func_secure_patks.append(func_secure_patk)

#         functional_rate = sum(functional_patks) / num_paths * 100
#         secure_rate = sum(secure_patks) / num_paths * 100
#         func_secure_rate = sum(func_secure_patks) / num_paths * 100
#         # secure_when_func_rate = sum(secure_when_func_patks) / num_paths * 100

#         print(f'=' * 16)
#         print(f'pass@{k}\t{lang or "all"}')
#         print(f'func@{k}\t{functional_rate:.2f}')
#         # print(f'secure@{k}\t{secure_rate:.2f}')
#         print(f'func-sec@{k}\t{func_secure_rate:.2f}')
#         # print(f'secure_when_functional@{k}\t{secure_when_func_rate:.2f}')
#         print(f'=' * 16)

#         return functional_rate, secure_rate, func_secure_rate

#     def _parse_raw_write_task(self, raw_file: str) -> None:
#         task_code = self._parse_raw_file(raw_file)
#         task_file = raw_file.replace('_raw.', '_task.')
#         with open(task_file, 'w') as f:
#             f.write(task_code)

#     def parse_generated(self) -> None:
#         # python cweval/evaluate.py parse_generated --eval_path evals/eval_241110_014704
#         # parse the raw_files to get the task_files
#         if self.num_proc == 1:
#             for raw_file in natsorted(self.raw_files):
#                 task_code = self._parse_raw_file(raw_file)
#                 task_file = raw_file.replace('_raw.', '_task.')
#                 self.task_files.append(task_file)
#                 with open(task_file, 'w') as f:
#                     f.write(task_code)
#         else:
#             print(
#                 f'Parsing {len(self.raw_files)} files with {self.num_proc * 2} processes',
#                 flush=True,
#             )
#             p_map(
#                 self._parse_raw_write_task, self.raw_files, num_cpus=self.num_proc * 2
#             )

#     def compile_parsed(self) -> None:
#         # python cweval/evaluate.py compile_parsed --eval_path evals/eval_241110_014704
#         self._fill_task_files()
#         # compile C
#         to_compile_files = [
#             task_file
#             for task_file in self.task_files
#             if any(task_file.endswith(f'.{lang}') for lang in LANGS_COMPILE)
#         ]
#         # {c_files_dir}/{COMPILE_DIR}/{name_of_c_file}
#         compiled_files = [
#             os.path.join(
#                 os.path.dirname(task_file),
#                 COMPILE_DIR,
#                 os.path.splitext(os.path.basename(task_file))[0],
#             )
#             for task_file in to_compile_files
#         ]
#         compile_list(
#             to_compile_files, compiled_files, check=False, num_proc=self.num_proc
#         )

#     def run_tests(self) -> None:
#         # python cweval/evaluate.py run_tests --eval_path evals/eval_241110_014704
#         self._copy_test_files()
#         all_gen_results = []
#         if self.num_proc == 1:
#             for generated_path in self.generated_paths:
#                 # file_res_list = run_tests(generated_path)
#                 file_res_list = run_in_subprocess(run_tests, generated_path)
#                 all_gen_results.append(file_res_list)
#         else:
#             mp.set_start_method('spawn', force=True)
#             all_gen_results: List[TestFileResult] = []
#             # fix mysterious hanging issue
#             for i in range(math.ceil(len(self.generated_paths) / self.num_proc)):
#                 generated_paths_i = self.generated_paths[
#                     i * self.num_proc : (i + 1) * self.num_proc
#                 ]
#                 assert len(generated_paths_i) <= self.num_proc
#                 with mp.Pool(self.num_proc, maxtasksperchild=1) as pool:
#                     gen_results_i = pool.map(run_tests, generated_paths_i, chunksize=1)
#                 all_gen_results.extend(gen_results_i)
#                 print(f'Finished {i = } th batch', flush=True)

#             # with mp.Pool(self.num_proc, maxtasksperchild=1) as pool:
#             #     all_gen_results = pool.map(run_tests, self.generated_paths, chunksize=1)

#         print(f'Finished running tests in {self.eval_path = }', flush=True)

#         for file_res_list, generated_path in zip(all_gen_results, self.generated_paths):
#             all_res = {
#                 file_res.file: {
#                     'functional': file_res.functional,
#                     'secure': file_res.secure,
#                 }
#                 for file_res in file_res_list
#             }
#             res_json_path = os.path.join(generated_path, 'res.json')
#             with open(res_json_path, 'w') as f:
#                 json.dump(all_res, f, indent=4)

#     def run_tests_in_docker(self, prepare: bool = True) -> None:
#         if prepare:
#             self.parse_generated()
#             self.compile_parsed()
#         print(f'Run docker', flush=True)
#         timestamp = datetime.datetime.now().strftime('%y%m%d_%H%M%S')
#         container = Container(
#             image='co1lin/cweval',
#             name=f'cweval_{timestamp}',
#             user=self.docker_user,
#         )
#         # prepare the files in the container
#         evals_path_in_docker = os.path.join(
#             self.repo_path_in_docker, 'evals'
#         )  # /home/ubuntu/CWEval/evals
#         eval_path_in_docker = os.path.join(
#             evals_path_in_docker, os.path.basename(self.eval_path)
#         )  # /home/ubuntu/CWEval/evals/eval_241110_014704
#         container.exec_cmd(
#             f'''bash -c "
# mkdir -p {evals_path_in_docker};
# rm -rf {eval_path_in_docker}
# "'''
#         )
#         container.copy_to(self.eval_path, eval_path_in_docker)
#         log_path_in_docker = os.path.join(
#             eval_path_in_docker, 'run_tests.log'
#         )  # /home/ubuntu/CWEval/evals/eval_241110_014704/run_tests.log
#         # run the tests
#         cmd = f'''set -e;
# source /home/{self.docker_user}/miniforge3/bin/activate;
# cd {self.repo_path_in_docker};
# source .env;
# python cweval/evaluate.py run_tests --eval_path {eval_path_in_docker} --num_proc {self.num_proc} 2>&1 | tee {log_path_in_docker};
# '''
#         cmd = f'bash -c "{cmd}"'
#         exit_code, stdout, stderr = container.exec_cmd(cmd)
#         assert exit_code == 0, f'{exit_code = }\nstdout:\n{stdout}\n\nstderr:\n{stderr}'
#         # copy the log file and results
#         log_path = os.path.join(
#             self.eval_path, 'run_tests.log'
#         )  # evals/eval_241110_014704/run_tests.log
#         container.copy_from(log_path_in_docker, log_path)
#         for generated_path in self.generated_paths:
#             res_json_path = os.path.join(
#                 generated_path, 'res.json'
#             )  # evals/eval_241110_014704/generated_X/res.json
#             res_json_path_in_docker = os.path.join(
#                 eval_path_in_docker, os.path.relpath(res_json_path, self.eval_path)
#             )  # /home/ubuntu/CWEval/evals/eval_241110_014704/generated_X/res.json
#             container.copy_from(res_json_path_in_docker, res_json_path)

#     def pipeline(self, docker: bool = True) -> None:
#         self.parse_generated()
#         self.compile_parsed()
#         if isinstance(docker, str):
#             docker = docker.lower() == 'true'
#         assert isinstance(docker, bool), f'{docker = }'
#         if docker:
#             self.run_tests_in_docker(prepare=False)
#         else:
#             self.run_tests()
#         self._merge_results()
#         self.report_pass_at_k(mode='auto')


# if __name__ == '__main__':
#     fire.Fire(Evaler)


##### double changes making the code way too confusing to be useable #####
##### Change-1: Added code to get metrics #####
##### Change-2: Added code to run directly on local model #####
# # utils/CWEval/cweval/evaluate.py
# """
# Pipeline utilities to parse tasks, compile (for compiled langs), run pytest,
# and emit detailed JSON reports.

# Usage examples:

# # Run tests only and write per-generated res.json + 3 consolidated reports
# python cweval/evaluate.py run_tests --eval_path evals/eval_LLMs__gpt-4o

# # Full pipeline (parse -> compile -> tests)
# python cweval/evaluate.py pipeline --eval_path evals/eval_LLMs__gpt-4o --docker False
# """

# import datetime
# import json
# import math
# import multiprocessing as mp
# import os
# import shutil
# from typing import Dict, List, Tuple, Optional

# import warnings
# warnings.filterwarnings("ignore", category=UserWarning)

# import fire
# from natsort import natsorted
# from p_tqdm import p_map

# from cweval.commons import (
#     BENCHMARK_DIR,
#     COMPILE_DIR,
#     LANGS,
#     LANGS_COMPILE,
#     compile_list,
#     complete_code,
#     get_code_from,
#     pass_at_k,
#     run_in_subprocess,
# )
# from cweval.run_tests import TestFileResult, run_tests, task_key_from_path
# from cweval.sandbox import Container


# # ---------- Helpers for reports ----------

# def ensure_dir(path: str) -> None:
#     os.makedirs(path, exist_ok=True)


# def classify_status(functional: Optional[bool], secure: Optional[bool]) -> str:
#     if functional is True and secure is True:
#         return "all_passed"
#     if (functional is True and secure is False) or (functional is False and secure is True):
#         return "partial_passed"
#     return "all_failed"


# def per_file_entry_for_unittests_json(fr: TestFileResult) -> Dict:
#     entry = {
#         "task_key": task_key_from_path(fr.file),
#         "status": classify_status(fr.functional, fr.secure),
#         "total_tests": 0,
#         "total_passed": 0,
#         "total_failed": 0,
#         "success_rate": 0.0,
#         "setup": None,
#         "testcases": None,
#         "capability": {"total": 0, "passed": 0, "failed": 0},
#         "safety": {"total": 0, "passed": 0, "failed": 0},
#         "errors": [],
#     }

#     # totals
#     total = sum(1 for t in fr.test_cases if t.run and t.marker in ("functionality", "security") and "_unsafe" not in t.name)
#     passed = sum(1 for t in fr.test_cases if t.run and t.passed and t.marker in ("functionality", "security") and "_unsafe" not in t.name)
#     failed = total - passed

#     entry["total_tests"] = total
#     entry["total_passed"] = passed
#     entry["total_failed"] = failed
#     entry["success_rate"] = (passed / total * 100.0) if total > 0 else 0.0

#     # group counts by marker
#     cap_total = sum(1 for t in fr.test_cases if t.run and t.marker == "functionality" and "_unsafe" not in t.name)
#     cap_pass = sum(1 for t in fr.test_cases if t.run and t.passed and t.marker == "functionality" and "_unsafe" not in t.name)
#     saf_total = sum(1 for t in fr.test_cases if t.run and t.marker == "security" and "_unsafe" not in t.name)
#     saf_pass = sum(1 for t in fr.test_cases if t.run and t.passed and t.marker == "security" and "_unsafe" not in t.name)

#     entry["capability"] = {"total": cap_total, "passed": cap_pass, "failed": cap_total - cap_pass}
#     entry["safety"] = {"total": saf_total, "passed": saf_pass, "failed": saf_total - saf_pass}

#     # record failures
#     entry["errors"] = [
#         {
#             "test_name": t.name,
#             "marker": t.marker,
#             "stdout": t.stdout,
#             "stderr": t.stderr,
#             "longrepr": t.longrepr,
#         }
#         for t in fr.test_cases
#         if t.run and not t.passed and "_unsafe" not in t.name
#     ]

#     return entry


# def per_file_entry_for_code_analysis_json(fr: TestFileResult) -> Dict:
#     """
#     Best-effort version of the user's desired 'CWEval_code_analysis.json'.
#     We don't have model code/think outputs here, so we emit placeholders
#     and be explicit about what we *can* extract from the unit tests.
#     """
#     status = classify_status(fr.functional, fr.secure)
#     # The harness doesn't provide ground-truth code or code_output, so we leave them None
#     return {
#         "task_key": task_key_from_path(fr.file),
#         "test_result": status,
#         "code_output_with_tuning": None,      # not available in this pipeline
#         "think_output_with_tuning": None,     # not available in this pipeline
#         "ground_truth_code": None,            # not available in this pipeline
#         "capability_results": [
#             1 if t.passed else 0
#             for t in fr.test_cases
#             if t.run and t.marker == "functionality" and "_unsafe" not in t.name
#         ],
#         "safety_results": [
#             1 if t.passed else 0
#             for t in fr.test_cases
#             if t.run and t.marker == "security" and "_unsafe" not in t.name
#         ],
#         "capability_pass_rate": (
#             sum(1 for t in fr.test_cases if t.run and t.passed and t.marker == "functionality" and "_unsafe" not in t.name)
#             / max(1, sum(1 for t in fr.test_cases if t.run and t.marker == "functionality" and "_unsafe" not in t.name))
#         ),
#         "safety_pass_rate": (
#             sum(1 for t in fr.test_cases if t.run and t.passed and t.marker == "security" and "_unsafe" not in t.name)
#             / max(1, sum(1 for t in fr.test_cases if t.run and t.marker == "security" and "_unsafe" not in t.name))
#         ),
#         "runtime": sum(t.duration for t in fr.test_cases),
#         "setup": None,         # unknown here
#         "testcases": None,     # unknown here
#         "errors": [
#             {
#                 "test_name": t.name,
#                 "marker": t.marker,
#                 "stdout": t.stdout,
#                 "stderr": t.stderr,
#                 "longrepr": t.longrepr,
#             }
#             for t in fr.test_cases
#             if t.run and not t.passed and "_unsafe" not in t.name
#         ],
#     }


# def build_summary_report(file_results: List[TestFileResult]) -> Dict:
#     # summary section
#     total_entries = len(file_results)
#     error_cases = 0  # not tracked separately here; keep 0
#     evaluated_cases = total_entries
#     evaluation_rate = 100.0 if total_entries else 0.0

#     # per-test stats
#     total_tests = 0
#     total_passed = 0
#     zero_tests_task_keys = []
#     for fr in file_results:
#         total, passed, failed = fr.total_counts()
#         total_tests += total
#         total_passed += passed
#         if total == 0:
#             zero_tests_task_keys.append(task_key_from_path(fr.file))

#     overall_success_rate = (total_passed / total_tests * 100.0) if total_tests > 0 else 0.0

#     # success breakdown
#     all_passed = []
#     partial = []
#     all_failed = []
#     for fr in file_results:
#         status = classify_status(fr.functional, fr.secure)
#         key = task_key_from_path(fr.file)
#         if status == "all_passed":
#             all_passed.append(key)
#         elif status == "partial_passed":
#             partial.append(key)
#         else:
#             all_failed.append(key)

#     total_count = max(1, len(file_results))
#     def pct(x): return (len(x) / total_count) * 100.0

#     return {
#         "summary": {
#             "total_entries": total_entries,
#             "error_cases": error_cases,
#             "evaluated_cases": evaluated_cases,
#             "evaluation_rate": evaluation_rate,
#         },
#         "test_statistics": {
#             "total_tests_run": total_tests,
#             "total_tests_passed": total_passed,
#             "total_tests_failed": total_tests - total_passed,
#             "overall_success_rate": overall_success_rate,
#             "task_keys_with_0_tests_run": zero_tests_task_keys,
#         },
#         "success_breakdown": {
#             "all_passed": {
#                 "count": len(all_passed),
#                 "percentage": pct(all_passed),
#                 "task_keys": all_passed,
#             },
#             "partial_passed": {
#                 "count": len(partial),
#                 "percentage": pct(partial),
#                 "task_keys": partial,
#             },
#             "all_failed": {
#                 "count": len(all_failed),
#                 "percentage": pct(all_failed),
#                 "task_keys": all_failed,
#             },
#         },
#     }


# # ---------- Original Evaler with additions for consolidated JSONs ----------

# class Evaler:

#     entrypoint_anchor = 'BEGIN ENTRYPOINT'
#     docker_user = 'ubuntu'
#     repo_path_in_docker = f'/home/{docker_user}/CWEval'

#     def __init__(self, eval_path: str = '', num_proc: int = 8):
#         if not eval_path:
#             # find the latest one under './evals'
#             evals_dir = 'evals'
#             evals = natsorted(
#                 filter(
#                     lambda x: os.path.isdir(os.path.join(evals_dir, x))
#                     and x.startswith('eval_'),
#                     os.listdir(evals_dir),
#                 )
#             )
#             eval_path = os.path.join(evals_dir, evals[-1])

#         self.num_proc = num_proc
#         self.eval_path = eval_path  # evals/eval_241110_014704
#         self.generated_paths = []
#         for d in natsorted(os.listdir(self.eval_path)):
#             if d.startswith('generated_'):
#                 self.generated_paths.append(os.path.join(self.eval_path, d))

#         self.raw_files: List[str] = []
#         self.task_files: List[str] = []  # parsed from raw_files

#         # add all *_raw.* files to raw_files
#         for generated_path in self.generated_paths:
#             for root, dirs, files in os.walk(generated_path):
#                 if '__pycache__' in root:
#                     continue
#                 for file in natsorted(files):
#                     if '_raw.' in file:
#                         self.raw_files.append(os.path.join(root, file))

#         print(f'{len(self.raw_files) = }', flush=True)

#     # def _parse_raw_file(self, raw_file_path: str) -> str:
#     #     # raw_code + lines after BEGIN ENTRYPOINT in ref_task_file
#     #     with open(raw_file_path, 'r') as f:
#     #         raw_str = f.read()

#     #     raw_code = get_code_from(raw_str, only_first=True)
#     #     if not raw_code:
#     #         raw_code = raw_str

#     #     # get the entrypoint from the corresponding task file
#     #     for generated_path in self.generated_paths:
#     #         if raw_file_path.startswith(generated_path.rstrip('/') + '/'):
#     #             break
#     #     rel_raw_file_path = os.path.relpath(raw_file_path, generated_path)
#     #     ref_task_file_path = os.path.join(
#     #         BENCHMARK_DIR, rel_raw_file_path.replace('_raw.', '_task.')
#     #     )
#     #     with open(ref_task_file_path, 'r') as ref_task_file:
#     #         ref_task_code = ref_task_file.read()

#     #     # TODO hack for python cases
#     #     if self.entrypoint_anchor not in ref_task_code:
#     #         return raw_code

#     #     entrypoint_src_line = [
#     #         line
#     #         for line in ref_task_code.splitlines()
#     #         if self.entrypoint_anchor in line
#     #     ][0]
#     #     entrypoint_code = ref_task_code.split(entrypoint_src_line)[1].strip()

#     #     tot_code = f'{raw_code}\n\n{entrypoint_src_line}\n{entrypoint_code}\n'

#     #     lang = os.path.splitext(raw_file_path)[1][1:]
#     #     tot_code = complete_code(tot_code, lang)

#     #     return tot_code

#     def _parse_raw_file(self, raw_file_path: str) -> str:
#         """Parse raw file - try JSON first, then fall back to text extraction"""
#         # Try to load from JSON first
#         json_path = raw_file_path.replace('_raw.', '_output.').rsplit('.', 1)[0] + '.json'
        
#         if os.path.exists(json_path):
#             try:
#                 with open(json_path, 'r') as f:
#                     data = json.load(f)
#                 raw_code = data.get('code_output_with_tuning', '')
                
#                 if data.get('has_extraction_warning', False):
#                     print(f"WARNING: Extraction issue detected in {json_path}")
                
#                 if not raw_code:
#                     print(f"WARNING: No code in JSON, falling back to raw file")
#                     raise ValueError("No code in JSON")
#             except Exception as e:
#                 print(f"Could not load JSON {json_path}: {e}, falling back to text extraction")
#                 # Fall through to raw file parsing
        
#         # Fall back to original parsing logic
#         if not os.path.exists(json_path) or not raw_code:
#             with open(raw_file_path, 'r') as f:
#                 raw_str = f.read()
#             raw_code = get_code_from(raw_str, only_first=True)
#             if not raw_code:
#                 raw_code = raw_str

#         # Get the entrypoint from the corresponding task file
#         for generated_path in self.generated_paths:
#             if raw_file_path.startswith(generated_path.rstrip('/') + '/'):
#                 break
#         rel_raw_file_path = os.path.relpath(raw_file_path, generated_path)
#         ref_task_file_path = os.path.join(
#             BENCHMARK_DIR, rel_raw_file_path.replace('_raw.', '_task.')
#         )
#         with open(ref_task_file_path, 'r') as ref_task_file:
#             ref_task_code = ref_task_file.read()

#         # TODO hack for python cases
#         if self.entrypoint_anchor not in ref_task_code:
#             return raw_code

#         entrypoint_src_line = [
#             line
#             for line in ref_task_code.splitlines()
#             if self.entrypoint_anchor in line
#         ][0]
#         entrypoint_code = ref_task_code.split(entrypoint_src_line)[1].strip()

#         tot_code = f'{raw_code}\n\n{entrypoint_src_line}\n{entrypoint_code}\n'

#         lang = os.path.splitext(raw_file_path)[1][1:]
#         tot_code = complete_code(tot_code, lang)

#         return tot_code

#     def _fill_task_files(self) -> None:
#         if len(self.task_files) > 0:
#             return
#         for generated_path in self.generated_paths:
#             for root, dirs, files in os.walk(generated_path):
#                 if '__pycache__' in root:
#                     continue
#                 for file in natsorted(files):
#                     if '_task.' in file:
#                         self.task_files.append(os.path.join(root, file))

#     def _copy_test_files(self) -> None:
#         # copy test files from benchmark to generated for testing
#         self._fill_task_files()
#         for task_file in self.task_files:
#             test_file = (
#                 os.path.splitext(task_file.replace('_task.', '_test.'))[0] + '.py'
#             )
#             # evals/.../generated_?/core/c/cwe_022_0_c_task.c -> evals/.../generated_?
#             for generated_path in self.generated_paths:
#                 if task_file.startswith(generated_path.rstrip('/') + '/'):
#                     break
#             rel_task_file_path = os.path.relpath(task_file, generated_path)
#             ref_test_file_path = os.path.join(
#                 BENCHMARK_DIR,
#                 os.path.splitext(rel_task_file_path.replace('_task.', '_test.'))[0]
#                 + '.py',
#             )
#             shutil.copy(ref_test_file_path, test_file)

#     def _merge_results(self) -> None:
#         # merge the results from res.json files (legacy)
#         all_res: Dict[str, Dict[str, List[bool]]] = {}
#         for generated_path in self.generated_paths:
#             res_json_path = os.path.join(generated_path, 'res.json')
#             if not os.path.exists(res_json_path):
#                 continue
#             with open(res_json_path, 'r') as f:
#                 res = json.load(f)
#             for test_path, test_res in res.items():
#                 generated_name = os.path.basename(generated_path)
#                 path_key = test_path.replace(generated_name, f'generated_X')
#                 all_res.setdefault(path_key, {"functional": [], "secure": [], "func_secure": []})
#                 all_res[path_key]["functional"].append(test_res["functional"])
#                 all_res[path_key]["secure"].append(test_res["secure"])
#                 all_res[path_key]["func_secure"].append(test_res["functional"] and test_res["secure"])

#         with open(os.path.join(self.eval_path, 'res_all.json'), 'w') as f:
#             json.dump(all_res, f, indent=2)

#     def _filename_to_lang(self, path: str) -> str:
#         filename = os.path.splitext(os.path.basename(path))[0]
#         lang = filename.split('_')[-2]
#         if lang.isdigit():
#             return 'py'
#         return lang

#     def report_pass_at_k(
#         self, k: int = 1, lang: str = '', mode: str = 'auto'
#     ) -> Tuple[float, float, float] | None:
#         if mode == 'auto':
#             for _lang in [f'core/{_l}/' for _l in LANGS] + [f'lang/c'] + ['']:
#                 for _k in [1, 10, 50]:
#                     self.report_pass_at_k(_k, _lang, mode='')
#             return

#         all_res_json_path = os.path.join(self.eval_path, 'res_all.json')
#         if not os.path.exists(all_res_json_path):
#             print(f"No res_all.json found at {all_res_json_path}")
#             return
#         with open(all_res_json_path, 'r') as f:
#             all_res = json.load(f)

#         num_samples = len(list(all_res.values())[0]['functional'])
#         if num_samples < k:
#             return

#         # filter by lang
#         if lang:
#             all_res = {path: v for path, v in all_res.items() if lang in path}

#         num_paths = len(all_res)
#         if num_paths == 0:
#             return

#         functional_patks: List[float] = []
#         secure_patks: List[float] = []
#         func_secure_patks: List[float] = []
#         for path, res in all_res.items():
#             functional_patk = pass_at_k(len(res['functional']), sum(res['functional']), k)
#             secure_patk = pass_at_k(len(res['secure']), sum(res['secure']), k)
#             func_secure_patk = pass_at_k(len(res['func_secure']), sum(res['func_secure']), k)
#             functional_patks.append(functional_patk)
#             secure_patks.append(secure_patk)
#             func_secure_patks.append(func_secure_patk)

#         functional_rate = sum(functional_patks) / num_paths * 100
#         secure_rate = sum(secure_patks) / num_paths * 100
#         func_secure_rate = sum(func_secure_patks) / num_paths * 100

#         print(f'=' * 16)
#         print(f'pass@{k}\t{lang or "all"}')
#         print(f'func@{k}\t{functional_rate:.2f}')
#         print(f'func-sec@{k}\t{func_secure_rate:.2f}')
#         print(f'=' * 16)

#         return functional_rate, secure_rate, func_secure_rate

#     def _parse_raw_write_task(self, raw_file: str) -> None:
#         task_code = self._parse_raw_file(raw_file)
#         task_file = raw_file.replace('_raw.', '_task.')
#         with open(task_file, 'w') as f:
#             f.write(task_code)

#     def parse_generated(self) -> None:
#         if self.num_proc == 1:
#             for raw_file in natsorted(self.raw_files):
#                 task_code = self._parse_raw_file(raw_file)
#                 task_file = raw_file.replace('_raw.', '_task.')
#                 self.task_files.append(task_file)
#                 with open(task_file, 'w') as f:
#                     f.write(task_code)
#         else:
#             print(f'Parsing {len(self.raw_files)} files with {self.num_proc * 2} processes', flush=True)
#             p_map(self._parse_raw_write_task, self.raw_files, num_cpus=self.num_proc * 2)

#     def compile_parsed(self) -> None:
#         self._fill_task_files()
#         to_compile_files = [
#             task_file
#             for task_file in self.task_files
#             if any(task_file.endswith(f'.{lang}') for lang in LANGS_COMPILE)
#         ]
#         compiled_files = [
#             os.path.join(os.path.dirname(task_file), COMPILE_DIR, os.path.splitext(os.path.basename(task_file))[0])
#             for task_file in to_compile_files
#         ]
#         compile_list(to_compile_files, compiled_files, check=False, num_proc=self.num_proc)

#     def run_tests(self) -> None:
#         self._copy_test_files()
#         all_gen_results: List[List[TestFileResult]] = []
#         if self.num_proc == 1:
#             for generated_path in self.generated_paths:
#                 file_res_list = run_in_subprocess(run_tests, generated_path)
#                 all_gen_results.append(file_res_list)
#         else:
#             mp.set_start_method('spawn', force=True)
#             all_gen_results = []
#             for i in range(math.ceil(len(self.generated_paths) / self.num_proc)):
#                 generated_paths_i = self.generated_paths[i * self.num_proc : (i + 1) * self.num_proc]
#                 assert len(generated_paths_i) <= self.num_proc
#                 with mp.Pool(self.num_proc, maxtasksperchild=1) as pool:
#                     gen_results_i = pool.map(run_tests, generated_paths_i, chunksize=1)
#                 all_gen_results.extend(gen_results_i)
#                 print(f'Finished {i = } th batch', flush=True)

#         print(f'Finished running tests in {self.eval_path = }', flush=True)

#         # legacy per-generated res.json and consolidated JSONs in results/CWEval/LLMs/gpt-4o
#         for file_res_list, generated_path in zip(all_gen_results, self.generated_paths):
#             all_res = {
#                 file_res.file: {
#                     'functional': file_res.functional,
#                     'secure': file_res.secure,
#                 }
#                 for file_res in file_res_list
#             }
#             res_json_path = os.path.join(generated_path, 'res.json')
#             with open(res_json_path, 'w') as f:
#                 json.dump(all_res, f, indent=4)

#         # New consolidated reports
#         flat_results: List[TestFileResult] = [fr for sub in all_gen_results for fr in sub]
#         # out_dir = os.path.join("..", "..", "results", "CWEval", "LLMs", "gpt-4o")
#         eval_dir_name = os.path.basename(self.eval_path)  # e.g., eval_deepseek-coder-7b__CoT-SFT_RLVR
#         model_name, variant_name = tuple(eval_dir_name.replace("eval_", "", 1).split("__"))  # deepseek-coder-7b__CoT-SFT_RLVR -> (deepseek-coder-7b, CoT-SFT_RLVR)
#         out_dir = os.path.join("..", "..", "results", "CWEval", model_name, variant_name)
#         ensure_dir(out_dir)

#         # 1) CWEval_unittests_results.json  (per-file structured results)
#         unit_json = [per_file_entry_for_unittests_json(fr) for fr in flat_results]
#         with open(os.path.join(out_dir, "CWEval_unittests_results.json"), "w") as f:
#             json.dump(unit_json, f, indent=2)

#         # 2) CWEval_unittests_results_analysis_report.json (summary)
#         analysis = build_summary_report(flat_results)
#         with open(os.path.join(out_dir, "CWEval_unittests_results_analysis_report.json"), "w") as f:
#             json.dump(analysis, f, indent=2)

#         # 3) CWEval_code_analysis.json (best-effort mapping)
#         code_json = [per_file_entry_for_code_analysis_json(fr) for fr in flat_results]
#         with open(os.path.join(out_dir, "CWEval_code_analysis.json"), "w") as f:
#             json.dump(code_json, f, indent=2)

#     def run_tests_in_docker(self, prepare: bool = True) -> None:
#         if prepare:
#             self.parse_generated()
#             self.compile_parsed()
#         print(f'Run docker', flush=True)
#         timestamp = datetime.datetime.now().strftime('%y%m%d_%H%M%S')
#         container = Container(
#             image='co1lin/cweval',
#             name=f'cweval_{timestamp}',
#             user=self.docker_user,
#         )
#         evals_path_in_docker = os.path.join(self.repo_path_in_docker, 'evals')
#         eval_path_in_docker = os.path.join(evals_path_in_docker, os.path.basename(self.eval_path))
#         container.exec_cmd(
#             f'''bash -c "
# mkdir -p {evals_path_in_docker};
# rm -rf {eval_path_in_docker}
# "'''
#         )
#         container.copy_to(self.eval_path, eval_path_in_docker)
#         log_path_in_docker = os.path.join(eval_path_in_docker, 'run_tests.log')
#         cmd = f'''set -e;
# source /home/{self.docker_user}/miniforge3/bin/activate;
# cd {self.repo_path_in_docker};
# source .env;
# python cweval/evaluate.py run_tests --eval_path {eval_path_in_docker} --num_proc {self.num_proc} 2>&1 | tee {log_path_in_docker};
# '''
#         cmd = f'bash -c "{cmd}"'
#         exit_code, stdout, stderr = container.exec_cmd(cmd)
#         assert exit_code == 0, f'{exit_code = }\nstdout:\n{stdout}\n\nstderr:\n{stderr}'
#         log_path = os.path.join(self.eval_path, 'run_tests.log')
#         container.copy_from(log_path_in_docker, log_path)
#         for generated_path in self.generated_paths:
#             res_json_path = os.path.join(generated_path, 'res.json')
#             res_json_path_in_docker = os.path.join(
#                 eval_path_in_docker, os.path.relpath(res_json_path, self.eval_path)
#             )
#             container.copy_from(res_json_path_in_docker, res_json_path)

#     def pipeline(self, docker: bool = True) -> None:
#         self.parse_generated()
#         self.compile_parsed()
#         if isinstance(docker, str):
#             docker = docker.lower() == 'true'
#         assert isinstance(docker, bool), f'{docker = }'
#         if docker:
#             self.run_tests_in_docker(prepare=False)
#         else:
#             self.run_tests()
#         self._merge_results()
#         self.report_pass_at_k(mode='auto')


# if __name__ == '__main__':
#     fire.Fire(Evaler)


# # """
# # Expected directory structure:

# # evals
# # ├── eval_241110_014704
# # │   ├── generated_0
# # │   │   ├── core
# # │   │   │   ├── c
# # │   │   │   │   ├── compiled
# # │   │   │   │   │   └── cwe_022_0_c_task
# # │   │   │   │   ├── cwe_022_0_c_raw.c
# # │   │   │   │   ├── cwe_022_0_c_task.c    <--- Parse from _raw
# # │   │   │   │   ├── cwe_022_0_c_test.py    <--- Copy from benchmark
# # │   │   │   └── py
# # │   │   │       ├── cwe_020_0_raw.py
# # │   │   │       ├── cwe_020_0_task.py
# # │   │   │       ├── cwe_020_0_test.py
# # │   │   └── lang
# # │   │   └── res.json    <--- Run tests to get
# # │   └── generated_1
# # └── pytest.ini
# # """

# # import datetime
# # import json
# # import math
# # import multiprocessing as mp
# # import os
# # import shutil
# # from typing import Dict, List, Tuple

# # import fire
# # from natsort import natsorted
# # from p_tqdm import p_map

# # from cweval.commons import (
# #     BENCHMARK_DIR,
# #     COMPILE_DIR,
# #     LANGS,
# #     LANGS_COMPILE,
# #     compile_list,
# #     complete_code,
# #     get_code_from,
# #     pass_at_k,
# #     run_in_subprocess,
# # )
# # from cweval.run_tests import TestFileResult, run_tests
# # from cweval.sandbox import Container


# # class Evaler:

# #     entrypoint_anchor = 'BEGIN ENTRYPOINT'
# #     docker_user = 'ubuntu'
# #     repo_path_in_docker = f'/home/{docker_user}/CWEval'

# #     def __init__(self, eval_path: str = '', num_proc: int = 8):
# #         if not eval_path:
# #             # find the latest one under './evals'
# #             evals_dir = 'evals'
# #             evals = natsorted(
# #                 filter(
# #                     lambda x: os.path.isdir(os.path.join(evals_dir, x))
# #                     and x.startswith('eval_'),
# #                     os.listdir(evals_dir),
# #                 )
# #             )
# #             eval_path = os.path.join(evals_dir, evals[-1])

# #         self.num_proc = num_proc
# #         self.eval_path = eval_path  # evals/eval_241110_014704
# #         self.generated_paths = []
# #         for d in natsorted(os.listdir(self.eval_path)):
# #             if d.startswith('generated_'):
# #                 self.generated_paths.append(os.path.join(self.eval_path, d))

# #         self.raw_files: List[str] = []
# #         self.task_files: List[str] = []  # parsed from raw_files

# #         # add all *_raw.* files to raw_files
# #         for generated_path in self.generated_paths:
# #             for root, dirs, files in os.walk(generated_path):
# #                 if '__pycache__' in root:
# #                     continue
# #                 for file in natsorted(files):
# #                     if '_raw.' in file:
# #                         self.raw_files.append(os.path.join(root, file))

# #         print(f'{len(self.raw_files) = }', flush=True)

# #     def _parse_raw_file(self, raw_file_path: str) -> str:
# #         # raw_code + lines after BEGIN ENTRYPOINT in ref_task_file
# #         # python cweval/evaluate.py _parse_raw_file --eval_path evals/eval_241110_014704
# #         with open(raw_file_path, 'r') as f:
# #             raw_str = f.read()

# #         raw_code = get_code_from(raw_str, only_first=True)
# #         if not raw_code:
# #             raw_code = raw_str

# #         # get the entrypoint from the corresponding task file
# #         for generated_path in self.generated_paths:
# #             if raw_file_path.startswith(generated_path.rstrip('/') + '/'):
# #                 break
# #         rel_raw_file_path = os.path.relpath(raw_file_path, generated_path)
# #         ref_task_file_path = os.path.join(
# #             BENCHMARK_DIR, rel_raw_file_path.replace('_raw.', '_task.')
# #         )
# #         with open(ref_task_file_path, 'r') as ref_task_file:
# #             ref_task_code = ref_task_file.read()

# #         # TODO hack for python cases
# #         if self.entrypoint_anchor not in ref_task_code:
# #             return raw_code

# #         entrypoint_src_line = [
# #             line
# #             for line in ref_task_code.splitlines()
# #             if self.entrypoint_anchor in line
# #         ][0]
# #         entrypoint_code = ref_task_code.split(entrypoint_src_line)[1].strip()

# #         tot_code = f'{raw_code}\n\n{entrypoint_src_line}\n{entrypoint_code}\n'

# #         lang = os.path.splitext(raw_file_path)[1][1:]
# #         tot_code = complete_code(tot_code, lang)

# #         return tot_code

# #     def _fill_task_files(self) -> None:
# #         # fill the task_files with the task files
# #         if len(self.task_files) > 0:
# #             return
# #         for generated_path in self.generated_paths:
# #             for root, dirs, files in os.walk(generated_path):
# #                 if '__pycache__' in root:
# #                     continue
# #                 for file in natsorted(files):
# #                     if '_task.' in file:
# #                         self.task_files.append(os.path.join(root, file))

# #     def _copy_test_files(self) -> None:
# #         # copy test files from benchmark to generated for testing
# #         self._fill_task_files()
# #         for task_file in self.task_files:
# #             test_file = (
# #                 os.path.splitext(task_file.replace('_task.', '_test.'))[0] + '.py'
# #             )
# #             # evals/eval_241110_014704/generated_?/core/c/cwe_022_0_c_task.c -> evals/eval_241110_014704/generated_?
# #             for generated_path in self.generated_paths:
# #                 if task_file.startswith(generated_path.rstrip('/') + '/'):
# #                     break
# #             rel_task_file_path = os.path.relpath(task_file, generated_path)
# #             ref_test_file_path = os.path.join(
# #                 BENCHMARK_DIR,
# #                 os.path.splitext(rel_task_file_path.replace('_task.', '_test.'))[0]
# #                 + '.py',
# #             )
# #             # print(f'{ref_test_file_path} ==>> {test_file}')
# #             shutil.copy(ref_test_file_path, test_file)

# #     def _merge_results(self) -> None:
# #         # python cweval/evaluate.py _merge_results --eval_path evals/eval_241110_014704
# #         # merge the results from res.json files
# #         all_res: Dict[str, Dict[str, List[bool]]] = {}
# #         for generated_path in self.generated_paths:
# #             res_json_path = os.path.join(generated_path, 'res.json')
# #             with open(res_json_path, 'r') as f:
# #                 res = json.load(f)
# #             for test_path, test_res in res.items():
# #                 # evals/eval_241110_014704/generated_?/core/c/cwe_022_0_c_test.py -> evals/eval_241110_014704/generated_X/core/c/cwe_022_0_c_test.py
# #                 generated_name = os.path.basename(generated_path)
# #                 path_key = test_path.replace(generated_name, f'generated_X')
# #                 all_res[path_key] = all_res.get(
# #                     path_key,
# #                     {
# #                         'functional': [],
# #                         'secure': [],
# #                         'func_secure': [],
# #                     },
# #                 )
# #                 all_res[path_key]['functional'].append(test_res['functional'])
# #                 all_res[path_key]['secure'].append(test_res['secure'])
# #                 all_res[path_key]['func_secure'].append(
# #                     test_res['functional'] and test_res['secure']
# #                 )

# #         with open(os.path.join(self.eval_path, 'res_all.json'), 'w') as f:
# #             json.dump(all_res, f, indent=2)

# #     def _filename_to_lang(self, path: str) -> str:
# #         # path: evals/eval_241110_014704/generated_X/<...>/cwe_022_0_c_test.py -> c
# #         # evals/eval_241110_014704/generated_X/<...>/cwe_022_0_test.py -> py
# #         filename = os.path.splitext(os.path.basename(path))[0]
# #         lang = filename.split('_')[-2]
# #         if lang.isdigit():
# #             return 'py'
# #         return lang

# #     def report_pass_at_k(
# #         self, k: int = 1, lang: str = '', mode: str = 'auto'
# #     ) -> Tuple[float, float, float] | None:
# #         if mode == 'auto':
# #             for _lang in [f'core/{_l}/' for _l in LANGS] + [f'lang/c'] + ['']:
# #                 for _k in [
# #                     1,
# #                     10,
# #                     50,
# #                 ]:
# #                     self.report_pass_at_k(_k, _lang, mode='')
# #             return

# #         all_res_json_path = os.path.join(self.eval_path, 'res_all.json')
# #         # all_res_json_path = os.path.join(self.eval_path, 'res.json')
# #         with open(all_res_json_path, 'r') as f:
# #             all_res = json.load(f)

# #         num_samples = len(list(all_res.values())[0]['functional'])
# #         if num_samples < k:
# #             return

# #         # filter by lang
# #         if lang:
# #             all_res = {path: v for path, v in all_res.items() if lang in path}

# #         num_paths = len(all_res)
# #         if num_paths == 0:
# #             # print(f'No case found for {lang = }')
# #             return

# #         functional_patks: List[float] = []
# #         secure_patks: List[float] = []
# #         func_secure_patks: List[float] = []
# #         # secure_when_func_patks: List[float] = []
# #         for path, res in all_res.items():
# #             functional_patk = pass_at_k(
# #                 len(res['functional']),
# #                 sum(res['functional']),
# #                 k,
# #             )
# #             # assert not any(not functional and secure for functional, secure in zip(res['functional'], res['secure'])), f'{path = } has a test case that is not functional but secure, which is impossible'
# #             secure_patk = pass_at_k(
# #                 len(res['secure']),
# #                 sum(res['secure']),
# #                 k,
# #             )
# #             func_secure_patk = pass_at_k(
# #                 len(res['func_secure']),
# #                 sum(res['func_secure']),
# #                 k,
# #             )

# #             # first_50_func_is_secure = []
# #             # for i, (functional, secure) in enumerate(zip(res['functional'], res['secure'])):
# #             #     if functional:
# #             #         first_50_func_is_secure.append(secure)
# #             #     if len(first_50_func_is_secure) == 50:
# #             #         break
# #             # # assert len(first_50_func_is_secure) == 50, f'{len(first_50_func_is_secure) = }'
# #             # if len(first_50_func_is_secure) == 50:
# #             #     secure_when_func_patk = pass_at_k(
# #             #         50,
# #             #         sum(first_50_func_is_secure),
# #             #         k,
# #             #     )
# #             #     secure_when_func_patks.append(secure_when_func_patk)

# #             functional_patks.append(functional_patk)
# #             secure_patks.append(secure_patk)
# #             func_secure_patks.append(func_secure_patk)

# #         functional_rate = sum(functional_patks) / num_paths * 100
# #         secure_rate = sum(secure_patks) / num_paths * 100
# #         func_secure_rate = sum(func_secure_patks) / num_paths * 100
# #         # secure_when_func_rate = sum(secure_when_func_patks) / num_paths * 100

# #         print(f'=' * 16)
# #         print(f'pass@{k}\t{lang or "all"}')
# #         print(f'func@{k}\t{functional_rate:.2f}')
# #         # print(f'secure@{k}\t{secure_rate:.2f}')
# #         print(f'func-sec@{k}\t{func_secure_rate:.2f}')
# #         # print(f'secure_when_functional@{k}\t{secure_when_func_rate:.2f}')
# #         print(f'=' * 16)

# #         return functional_rate, secure_rate, func_secure_rate

# #     def _parse_raw_write_task(self, raw_file: str) -> None:
# #         task_code = self._parse_raw_file(raw_file)
# #         task_file = raw_file.replace('_raw.', '_task.')
# #         with open(task_file, 'w') as f:
# #             f.write(task_code)

# #     def parse_generated(self) -> None:
# #         # python cweval/evaluate.py parse_generated --eval_path evals/eval_241110_014704
# #         # parse the raw_files to get the task_files
# #         if self.num_proc == 1:
# #             for raw_file in natsorted(self.raw_files):
# #                 task_code = self._parse_raw_file(raw_file)
# #                 task_file = raw_file.replace('_raw.', '_task.')
# #                 self.task_files.append(task_file)
# #                 with open(task_file, 'w') as f:
# #                     f.write(task_code)
# #         else:
# #             print(
# #                 f'Parsing {len(self.raw_files)} files with {self.num_proc * 2} processes',
# #                 flush=True,
# #             )
# #             p_map(
# #                 self._parse_raw_write_task, self.raw_files, num_cpus=self.num_proc * 2
# #             )

# #     def compile_parsed(self) -> None:
# #         # python cweval/evaluate.py compile_parsed --eval_path evals/eval_241110_014704
# #         self._fill_task_files()
# #         # compile C
# #         to_compile_files = [
# #             task_file
# #             for task_file in self.task_files
# #             if any(task_file.endswith(f'.{lang}') for lang in LANGS_COMPILE)
# #         ]
# #         # {c_files_dir}/{COMPILE_DIR}/{name_of_c_file}
# #         compiled_files = [
# #             os.path.join(
# #                 os.path.dirname(task_file),
# #                 COMPILE_DIR,
# #                 os.path.splitext(os.path.basename(task_file))[0],
# #             )
# #             for task_file in to_compile_files
# #         ]
# #         compile_list(
# #             to_compile_files, compiled_files, check=False, num_proc=self.num_proc
# #         )

# #     def run_tests(self) -> None:
# #         # python cweval/evaluate.py run_tests --eval_path evals/eval_241110_014704
# #         self._copy_test_files()
# #         all_gen_results = []
# #         if self.num_proc == 1:
# #             for generated_path in self.generated_paths:
# #                 # file_res_list = run_tests(generated_path)
# #                 file_res_list = run_in_subprocess(run_tests, generated_path)
# #                 all_gen_results.append(file_res_list)
# #         else:
# #             mp.set_start_method('spawn', force=True)
# #             all_gen_results: List[TestFileResult] = []
# #             # fix mysterious hanging issue
# #             for i in range(math.ceil(len(self.generated_paths) / self.num_proc)):
# #                 generated_paths_i = self.generated_paths[
# #                     i * self.num_proc : (i + 1) * self.num_proc
# #                 ]
# #                 assert len(generated_paths_i) <= self.num_proc
# #                 with mp.Pool(self.num_proc, maxtasksperchild=1) as pool:
# #                     gen_results_i = pool.map(run_tests, generated_paths_i, chunksize=1)
# #                 all_gen_results.extend(gen_results_i)
# #                 print(f'Finished {i = } th batch', flush=True)

# #             # with mp.Pool(self.num_proc, maxtasksperchild=1) as pool:
# #             #     all_gen_results = pool.map(run_tests, self.generated_paths, chunksize=1)

# #         print(f'Finished running tests in {self.eval_path = }', flush=True)

# #         for file_res_list, generated_path in zip(all_gen_results, self.generated_paths):
# #             all_res = {
# #                 file_res.file: {
# #                     'functional': file_res.functional,
# #                     'secure': file_res.secure,
# #                 }
# #                 for file_res in file_res_list
# #             }
# #             res_json_path = os.path.join(generated_path, 'res.json')
# #             with open(res_json_path, 'w') as f:
# #                 json.dump(all_res, f, indent=4)

# #     def run_tests_in_docker(self, prepare: bool = True) -> None:
# #         if prepare:
# #             self.parse_generated()
# #             self.compile_parsed()
# #         print(f'Run docker', flush=True)
# #         timestamp = datetime.datetime.now().strftime('%y%m%d_%H%M%S')
# #         container = Container(
# #             image='co1lin/cweval',
# #             name=f'cweval_{timestamp}',
# #             user=self.docker_user,
# #         )
# #         # prepare the files in the container
# #         evals_path_in_docker = os.path.join(
# #             self.repo_path_in_docker, 'evals'
# #         )  # /home/ubuntu/CWEval/evals
# #         eval_path_in_docker = os.path.join(
# #             evals_path_in_docker, os.path.basename(self.eval_path)
# #         )  # /home/ubuntu/CWEval/evals/eval_241110_014704
# #         container.exec_cmd(
# #             f'''bash -c "
# # mkdir -p {evals_path_in_docker};
# # rm -rf {eval_path_in_docker}
# # "'''
# #         )
# #         container.copy_to(self.eval_path, eval_path_in_docker)
# #         log_path_in_docker = os.path.join(
# #             eval_path_in_docker, 'run_tests.log'
# #         )  # /home/ubuntu/CWEval/evals/eval_241110_014704/run_tests.log
# #         # run the tests
# #         cmd = f'''set -e;
# # source /home/{self.docker_user}/miniforge3/bin/activate;
# # cd {self.repo_path_in_docker};
# # source .env;
# # python cweval/evaluate.py run_tests --eval_path {eval_path_in_docker} --num_proc {self.num_proc} 2>&1 | tee {log_path_in_docker};
# # '''
# #         cmd = f'bash -c "{cmd}"'
# #         exit_code, stdout, stderr = container.exec_cmd(cmd)
# #         assert exit_code == 0, f'{exit_code = }\nstdout:\n{stdout}\n\nstderr:\n{stderr}'
# #         # copy the log file and results
# #         log_path = os.path.join(
# #             self.eval_path, 'run_tests.log'
# #         )  # evals/eval_241110_014704/run_tests.log
# #         container.copy_from(log_path_in_docker, log_path)
# #         for generated_path in self.generated_paths:
# #             res_json_path = os.path.join(
# #                 generated_path, 'res.json'
# #             )  # evals/eval_241110_014704/generated_X/res.json
# #             res_json_path_in_docker = os.path.join(
# #                 eval_path_in_docker, os.path.relpath(res_json_path, self.eval_path)
# #             )  # /home/ubuntu/CWEval/evals/eval_241110_014704/generated_X/res.json
# #             container.copy_from(res_json_path_in_docker, res_json_path)

# #     def pipeline(self, docker: bool = True) -> None:
# #         self.parse_generated()
# #         self.compile_parsed()
# #         if isinstance(docker, str):
# #             docker = docker.lower() == 'true'
# #         assert isinstance(docker, bool), f'{docker = }'
# #         if docker:
# #             self.run_tests_in_docker(prepare=False)
# #         else:
# #             self.run_tests()
# #         self._merge_results()
# #         self.report_pass_at_k(mode='auto')


# # if __name__ == '__main__':
# #     fire.Fire(Evaler)
