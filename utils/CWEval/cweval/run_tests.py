# utils/CWEval/cweval/run_tests.py
import os
from dataclasses import dataclass, field, asdict
from typing import Dict, List, Optional

import pytest

CWD = os.getcwd()


@dataclass
class TestCaseResult:
    name: str
    marker: str  # "functionality" | "security"
    passed: bool
    run: bool
    duration: float = 0.0
    stdout: str = ""
    stderr: str = ""
    longrepr: Optional[str] = None  # short failure text if any


@dataclass
class TestFileResult:
    file: str  # path relative to repo root
    functional: Optional[bool]
    secure: Optional[bool]
    test_cases: List[TestCaseResult] = field(default_factory=list)

    def brief_str(self):
        return f'{__class__.__name__}(file="{self.file}", functional={self.functional}, secure={self.secure})'

    def total_counts(self):
        total = sum(1 for t in self.test_cases if t.run and t.marker in ("functionality", "security") and "_unsafe" not in t.name)
        passed = sum(1 for t in self.test_cases if t.run and t.passed and t.marker in ("functionality", "security") and "_unsafe" not in t.name)
        failed = total - passed
        return total, passed, failed

    def to_json(self) -> Dict:
        total, passed, failed = self.total_counts()
        # group counts by marker
        cap_total = sum(1 for t in self.test_cases if t.run and t.marker == "functionality" and "_unsafe" not in t.name)
        cap_passed = sum(1 for t in self.test_cases if t.run and t.passed and t.marker == "functionality" and "_unsafe" not in t.name)
        saf_total = sum(1 for t in self.test_cases if t.run and t.marker == "security" and "_unsafe" not in t.name)
        saf_passed = sum(1 for t in self.test_cases if t.run and t.passed and t.marker == "security" and "_unsafe" not in t.name)

        status = "all_failed"
        if self.functional is True and self.secure is True:
            status = "all_passed"
        elif (self.functional is True and self.secure is False) or (self.functional is False and self.secure is True):
            status = "partial_passed"
        elif self.functional is None and self.secure is None and total == 0:
            status = "all_failed"

        return {
            "task_key": task_key_from_path(self.file),
            "status": status,
            "total_tests": total,
            "total_passed": passed,
            "total_failed": failed,
            "success_rate": (passed / total * 100.0) if total > 0 else 0,
            "capability": {
                "total": cap_total,
                "passed": cap_passed,
                "failed": cap_total - cap_passed,
            },
            "safety": {
                "total": saf_total,
                "passed": saf_passed,
                "failed": saf_total - saf_passed,
            },
            "errors": [
                {
                    "test_name": t.name,
                    "marker": t.marker,
                    "reason": "assertion_failed" if t.run else "not_run",
                    "stdout": t.stdout,
                    "stderr": t.stderr,
                    "longrepr": t.longrepr,
                }
                for t in self.test_cases
                if t.run and not t.passed and "_unsafe" not in t.name
            ],
        }


def task_key_from_path(path: str) -> str:
    """
    Create a stable id-like key from a pytest file path.
    Example: evals/eval_LLMs__gpt-4o/generated_0/core/py/cwe_020_0_test.py
    -> cwe_020_0_py
    """
    base = os.path.basename(path)
    stem = os.path.splitext(base)[0]  # cwe_020_0_test
    parts = stem.split("_")
    # typical stems: cwe, 020, 0, test
    # try to insert language from parent folder if available
    lang = ""
    try:
        parent = os.path.basename(os.path.dirname(path))  # py
        if parent in ("py", "c", "cpp", "java", "go", "js", "ts"):
            lang = parent
    except Exception:
        pass

    # keep cwe_*_* and append lang
    if len(parts) >= 3 and parts[0] == "cwe":
        key = f"{parts[0]}_{parts[1]}_{parts[2]}"
        if lang:
            key = f"{key}_{lang}"
        return key
    return stem


class TestResultCollector:
    def __init__(self, timeout_per_test: float = 20):
        self.file_results: Dict[str, TestFileResult] = {}
        self.nodeid_to_test_case: Dict[str, TestCaseResult] = {}
        self.timeout_per_test = timeout_per_test

    def pytest_collection_modifyitems(self, session, config, items):
        """
        Collect test case details and mark timeouts.
        """
        for item in items:
            marker = None
            if item.get_closest_marker("functionality"):
                marker = "functionality"
            elif item.get_closest_marker("security"):
                marker = "security"
            else:
                continue

            # prevent hanging tests
            item.add_marker(pytest.mark.timeout(self.timeout_per_test, method="signal"))

            nodeid = item.nodeid  # like: <path>::<test_name>
            file_path, test_name = nodeid.split("::", 1)

            if file_path not in self.file_results:
                rel = os.path.relpath(item.path, CWD)
                self.file_results[file_path] = TestFileResult(
                    file=rel, functional=None, secure=None
                )

            test_case = TestCaseResult(
                name=test_name, marker=marker, passed=False, run=False
            )
            self.file_results[file_path].test_cases.append(test_case)
            self.nodeid_to_test_case[nodeid] = test_case

    def pytest_runtest_logreport(self, report):
        """
        Collect outcomes and diagnostics per test.
        """
        if report.when == "call":
            nodeid = report.nodeid
            test_case = self.nodeid_to_test_case.get(nodeid)
            if not test_case:
                return
            test_case.run = True
            test_case.passed = (report.outcome == "passed")
            # duration, stdout/stderr, and short fail
            try:
                test_case.duration = float(getattr(report, "duration", 0.0) or 0.0)
            except Exception:
                test_case.duration = 0.0
            test_case.stdout = getattr(report, "capstdout", "") or ""
            test_case.stderr = getattr(report, "capstderr", "") or ""
            if hasattr(report, "longreprtext") and report.outcome != "passed":
                test_case.longrepr = str(report.longreprtext)[:2000]


def run_tests(
    test_path: str,
    timeout_per_test: float = 3,
    args: List[str] = None,
) -> List[TestFileResult]:
    """
    Run pytest on `test_path` and return structured results.
    """
    print(f"Start running tests in {test_path = }", flush=True)
    if args is None:
        # '-k not _unsafe' = ignore helper “_unsafe” tests if present
        args = ["-k", "not _unsafe"]

    result_collector = TestResultCollector(timeout_per_test=timeout_per_test)

    # temp fix: prevent pytest from os._exit() on some plugins
    _os_exit = os._exit
    os._exit = lambda *a, **k: None
    try:
        pytest.main(
            [test_path, "--tb=short", "--continue-on-collection-errors", *args],
            plugins=[result_collector],
        )
    finally:
        os._exit = _os_exit

    print(f"[run_tests] Finished running tests in {test_path = }", flush=True)

    # compute file-level booleans
    for file_result in result_collector.file_results.values():
        file_result.functional = all(
            t.passed
            for t in file_result.test_cases
            if t.marker == "functionality" and "_unsafe" not in t.name
        )
        file_result.secure = all(
            t.passed
            for t in file_result.test_cases
            if t.marker == "security" and "_unsafe" not in t.name
        )

    return list(result_collector.file_results.values())


if __name__ == "__main__":
    results = run_tests("evals/eval_LLMs__gpt-4o")
    for r in results:
        print(r.brief_str())


# import os
# from dataclasses import dataclass, field
# from typing import Dict, List

# import pytest

# CWD = os.getcwd()


# @dataclass
# class TestCaseResult:
#     name: str
#     marker: str
#     passed: bool
#     run: bool


# @dataclass
# class TestFileResult:
#     file: str
#     functional: bool
#     secure: bool
#     test_cases: List[TestCaseResult] = field(default_factory=list)

#     def brief_str(self):
#         return f'{__class__.__name__}(file=\'{self.file}\', functional={self.functional}, secure={self.secure})'


# class TestResultCollector:
#     def __init__(self, timeout_per_test: float = 20):
#         # Dictionary to store results keyed by file path
#         self.file_results: Dict[str, TestFileResult] = {}
#         # Mapping from nodeid to TestCaseResult for quick lookup
#         self.nodeid_to_test_case: Dict[str, TestCaseResult] = {}
#         self.timeout_per_test = timeout_per_test

#     def pytest_collection_modifyitems(self, session, config, items):
#         """
#         Hook to collect test case details during the collection phase.
#         """
#         for item in items:
#             if item.get_closest_marker("functionality"):
#                 marker = "functionality"
#             elif item.get_closest_marker("security"):
#                 marker = "security"
#             else:
#                 continue
#             # prevent hanging tests
#             item.add_marker(pytest.mark.timeout(self.timeout_per_test, method="signal"))
#             # nodeid example: 'tests/test_file1.py::test_case_a'
#             nodeid = item.nodeid
#             # Extract file path and test name
#             file_path, test_name = nodeid.split("::", 1)
#             # Initialize TestFileResult if not already present
#             if file_path not in self.file_results:
#                 self.file_results[file_path] = TestFileResult(
#                     file=os.path.relpath(item.path, CWD), functional=None, secure=None
#                 )

#             # Create a TestCaseResult with default passed=False
#             test_case = TestCaseResult(
#                 name=test_name, marker=marker, passed=False, run=False
#             )
#             self.file_results[file_path].test_cases.append(test_case)

#             # Map nodeid to test_case_result for later reference
#             self.nodeid_to_test_case[nodeid] = test_case

#     def pytest_runtest_logreport(self, report):
#         """
#         Hook to collect the outcome of each test case.
#         """
#         if report.when == 'call':
#             nodeid = report.nodeid
#             test_case = self.nodeid_to_test_case.get(nodeid)
#             # if test_case:
#             test_case.run = True
#             test_case.passed = report.outcome == 'passed'
#             # print(test_case, flush=True)
#             # Update the TestFileResult's passed status
#             # file_path, _ = nodeid.split("::", 1)
#             # if not test_case.passed:
#             #     if test_case.marker == 'functionality':
#             #         self.file_results[file_path].functional = False
#             #     else:
#             #         self.file_results[file_path].secure = False


# def run_tests(
#     test_path,
#     timeout_per_test: float = 3,
#     args: List[str] = ['-k', 'not _unsafe'],
# ) -> List[TestFileResult]:
#     print(f'Start running tests in {test_path = }', flush=True)
#     result_collector = TestResultCollector(timeout_per_test=timeout_per_test)
#     # temp fix:
#     _os_exit = os._exit
#     os._exit = lambda *args: None
#     pytest.main(
#         [test_path, '--tb=short', '--continue-on-collection-errors', *args],
#         plugins=[result_collector],
#     )
#     os._exit = _os_exit
#     print(f'[run_tests] Finished running tests in {test_path = }', flush=True)
#     # compute file results
#     for file_result in result_collector.file_results.values():
#         # for test_case in file_result.test_cases:
#         #     is_unsafe = '_unsafe' in test_case.name
#         #     assert is_unsafe == (not test_case.run)
#         file_result.functional = all(
#             test_case.passed
#             for test_case in file_result.test_cases
#             if test_case.marker == 'functionality' and '_unsafe' not in test_case.name
#         )
#         file_result.secure = all(
#             test_case.passed
#             for test_case in file_result.test_cases
#             if test_case.marker == 'security' and '_unsafe' not in test_case.name
#         )
#         # print(file_result.brief_str(), flush=True)

#     return list(result_collector.file_results.values())


# if __name__ == "__main__":
#     results = run_tests("evals/eval_241110_014704")
#     for result in results:
#         print(result.brief_str())
