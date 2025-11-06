import os
import csv
import ast
import re
from pathlib import Path
from typing import Dict, List, Tuple, Optional
from dataclasses import dataclass, asdict


@dataclass
class CWEvalEntry:
    task_id: str
    id: str
    CWE_ID: str
    func_signature: str
    task_description: str
    safe_code: str
    vulnerable_code: str
    has_both_unit_tests: bool


def extract_function_info(task_file_content: str) -> Tuple[str, str, str]:
    """
    Extract function signature, docstring, and safe code from task file.
    
    Returns:
        Tuple of (function_signature, docstring, safe_implementation)
    """
    lines = task_file_content.split('\n')
    
    # Find the function definition
    func_start_idx = None
    for i, line in enumerate(lines):
        if line.strip().startswith('def '):
            func_start_idx = i
            break
    
    if func_start_idx is None:
        return "", "", ""
    
    # Extract function signature
    func_signature = lines[func_start_idx].strip()
    
    # Handle multi-line function signatures
    if '(' in func_signature and ')' not in func_signature:
        for i in range(func_start_idx + 1, len(lines)):
            func_signature += '\n' + lines[i].strip()
            if ')' in lines[i]:
                break
    
    # Extract docstring
    docstring = ""
    docstring_start = None
    for i in range(func_start_idx + 1, len(lines)):
        if "'''" in lines[i] or '"""' in lines[i]:
            if docstring_start is None:
                docstring_start = i
            else:
                # End of docstring
                docstring = '\n'.join(lines[docstring_start:i+1])
                break
    
    # Clean docstring
    docstring = docstring.strip()
    for quote in ["'''", '"""']:
        docstring = docstring.replace(quote, '')
    docstring = docstring.strip()
    
    # # Extract safe implementation (after BEGIN SOLUTION)
    # safe_code = ""
    # begin_solution_idx = None
    # for i, line in enumerate(lines):
    #     if 'BEGIN SOLUTION' in line:
    #         begin_solution_idx = i
    #         break
    
    # if begin_solution_idx is not None:
    #     safe_code = '\n'.join(lines[begin_solution_idx + 1:])
    #     safe_code = safe_code.strip()
    
    # Combine function signature with docstring for task description
    # task_description = func_signature
    task_description = docstring
    
    # return func_signature, task_description, safe_code
    return func_signature, task_description, task_file_content


def extract_unsafe_implementations(test_file_content: str) -> List[str]:
    """
    Extract all unsafe implementations from the test file.
    """
    unsafe_funcs = []
    lines = test_file_content.split('\n')
    
    current_func = []
    in_unsafe_func = False
    indent_level = 0
    
    for i, line in enumerate(lines):
        # Check if this is an unsafe function definition
        # e.g., def some_function_unsafe(...): but not def test_some_function_unsafe(...):
        pattern = r'^\s*def\s+(?!test_)\w+_unsafe'
        if re.match(pattern, line):
            in_unsafe_func = True
            current_func = [line]
            # Get the indentation level of the function
            indent_level = len(line) - len(line.lstrip())
            continue
        
        if in_unsafe_func:
            # Check if we've reached the next function or end of unsafe function
            if line.strip() and not line.startswith(' ' * (indent_level + 1)) and not line.strip().startswith('#'):
                if line.strip() and not line.startswith(' '):
                    # We've reached a new top-level definition
                    unsafe_funcs.append('\n'.join(current_func))
                    current_func = []
                    in_unsafe_func = False
            else:
                current_func.append(line)
    
    # Add the last function if exists
    if current_func:
        unsafe_funcs.append('\n'.join(current_func))
    
    return unsafe_funcs


def check_test_types(test_file_content: str) -> Tuple[bool, bool]:
    """
    Check if the test file has both functionality and security tests.
    
    Returns:
        Tuple of (has_functionality_tests, has_security_tests)
    """
    has_functionality = 'marks=pytest.mark.functionality' in test_file_content
    has_security = 'marks=pytest.mark.security' in test_file_content
    
    return has_functionality, has_security


def process_cwe_file(task_file_path: str, test_file_path: str) -> Optional[CWEvalEntry]:
    """
    Process a single CWE task and test file pair.
    """
    # Extract ID from filename (e.g., cwe_020_0_task.py -> cwe_020_0)
    filename = os.path.basename(task_file_path)
    file_id = filename.replace('_task.py', '')
    
    # Extract CWE ID (e.g., cwe_020_0 -> CWE-020)
    cwe_match = re.match(r'cwe_(\d+)_', file_id)
    if not cwe_match:
        return None
    cwe_id = f"CWE-{cwe_match.group(1)}"
    
    # Read task file
    try:
        with open(task_file_path, 'r', encoding='utf-8') as f:
            task_content = f.read()
    except Exception as e:
        print(f"Error reading task file {task_file_path}: {e}")
        return None
    
    # Read test file
    try:
        with open(test_file_path, 'r', encoding='utf-8') as f:
            test_content = f.read()
    except Exception as e:
        print(f"Error reading test file {test_file_path}: {e}")
        return None
    
    # Extract information
    func_signature, task_description, safe_code = extract_function_info(task_content)
    unsafe_implementations = extract_unsafe_implementations(test_content)
    has_func, has_sec = check_test_types(test_content)
    
    # Combine all unsafe implementations
    vulnerable_code = '\n\n'.join(unsafe_implementations) if unsafe_implementations else ""
    
    return CWEvalEntry(
        task_id=f"CWEval-{file_id}",
        id=file_id,
        CWE_ID=cwe_id,
        func_signature=func_signature,
        task_description=task_description,
        safe_code=safe_code,
        vulnerable_code=vulnerable_code,
        has_both_unit_tests=has_func and has_sec
    )


def process_benchmark_directory(benchmark_dir: str = "benchmark/core/py") -> List[CWEvalEntry]:
    """
    Process all CWE files in the benchmark directory.
    """
    entries = []
    
    # Find all task files
    task_files = []
    for root, dirs, files in os.walk(benchmark_dir):
        for file in files:
            if file.endswith('_task.py'):
                task_files.append(os.path.join(root, file))
    
    task_files.sort()
    
    for task_file in task_files:
        # Find corresponding test file
        test_file = task_file.replace('_task.py', '_test.py')
        
        if not os.path.exists(test_file):
            print(f"Warning: Test file not found for {task_file}")
            continue
        
        entry = process_cwe_file(task_file, test_file)
        if entry:
            entries.append(entry)
            print(f"Processed: {entry.id}")
        else:
            print(f"Failed to process: {task_file}")
    
    return entries


def save_to_csv(entries: List[CWEvalEntry], output_file: str = "cweval_dataset.csv"):
    """
    Save the processed entries to a CSV file.
    """
    if not entries:
        print("No entries to save!")
        return
    
    with open(output_file, 'w', newline='', encoding='utf-8') as f:
        writer = csv.DictWriter(f, fieldnames=asdict(entries[0]).keys())
        writer.writeheader()
        
        for entry in entries:
            writer.writerow(asdict(entry))
    
    print(f"\nSaved {len(entries)} entries to {output_file}")


def main():
    # Set the benchmark directory path
    # Adjust this path based on where you have the CWEval repository
    benchmark_dir = "CWEval-repo/benchmark/core/py"
    
    if not os.path.exists(benchmark_dir):
        print(f"Error: Benchmark directory not found: {benchmark_dir}")
        print("Please adjust the path to point to the CWEval benchmark directory")
        return
    
    print("Processing CWEval dataset...")
    entries = process_benchmark_directory(benchmark_dir)
    
    if entries:
        save_to_csv(entries, "data/cweval_dataset-2.csv")
        
        # Print summary statistics
        print(f"\n{'='*60}")
        print("Summary Statistics:")
        print(f"{'='*60}")
        print(f"Total entries: {len(entries)}")
        print(f"Entries with both test types: {sum(1 for e in entries if e.has_both_unit_tests)}")
        
        # Count by CWE
        cwe_counts = {}
        for entry in entries:
            cwe_counts[entry.CWE_ID] = cwe_counts.get(entry.CWE_ID, 0) + 1
        
        print(f"\nEntries by CWE:")
        for cwe_id in sorted(cwe_counts.keys()):
            print(f"  {cwe_id}: {cwe_counts[cwe_id]}")
    else:
        print("No entries were processed!")


if __name__ == "__main__":
    main()