#!/usr/bin/env python3
import argparse
import json
import os
import re
from pathlib import Path
from typing import Optional, Tuple

def read_json(path: Path) -> dict:
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)

CODE_BLOCK_PATTERN_MD = re.compile(
    r"```(?:python|c|cpp|go|javascript|js)?\s*(?P<code>[\s\S]*?)\s*```",
    flags=re.IGNORECASE
)
CODE_BLOCK_PATTERN_TAG = re.compile(
    r"<code>\s*(?P<code>[\s\S]*?)\s*</code>",
    flags=re.IGNORECASE
)
THINK_BLOCK_PATTERN = re.compile(
    r"<think>[\s\S]*?</think>",
    flags=re.IGNORECASE
)

# Language extension mapping
LANG_EXTENSIONS = {
    "c": ".c",
    "cpp": ".cpp",
    "go": ".go",
    "js": ".js",
    "py": ".py",
}

def extract_code(s: str) -> str:
    """
    Extract pure code from an LLM response string.
    Priority:
      1) <code> ... </code>
      2) ```(language)? ... ```
      3) fallback to raw string (after removing <think>...</think>)
    """
    if not isinstance(s, str):
        return ""

    # Remove <think> blocks entirely
    s_wo_think = THINK_BLOCK_PATTERN.sub("", s).strip()

    # Try <code> ... </code>
    m = CODE_BLOCK_PATTERN_TAG.search(s_wo_think)
    if m:
        return m.group("code").strip()

    # Try triple-backtick blocks
    m = CODE_BLOCK_PATTERN_MD.search(s_wo_think)
    if m:
        return m.group("code").strip()

    # Fallback: return remaining text as-is
    return s_wo_think.strip()

def parse_split(split: str) -> Tuple[str, str]:
    """
    Parse split field to extract directory path and language.
    Examples:
        "core_c" -> ("core", "c")
        "core_cpp" -> ("core", "cpp")
        "lang_c" -> ("lang", "c")
        "core_py" -> ("core", "py")
    
    Returns (subdir, language)
    """
    parts = split.rsplit("_", 1)
    if len(parts) == 2:
        return parts[0], parts[1]
    # Fallback if split doesn't match expected pattern
    return "core", "py"

def get_file_extension(language: str) -> str:
    """Get file extension for a given language."""
    return LANG_EXTENSIONS.get(language, ".txt")

def build_eval_dir_name(parent_path: str) -> str:
    """
    Convert 'deepseek-coder-7b/CoT-SFT_RLVR' -> 'eval_deepseek-coder-7b__CoT-SFT_RLVR'
    """
    safe = parent_path.strip("/").replace("/", "__")
    return f"eval_{safe}"

def write_code_file(eval_root: Path, task_id: str, code: str, split: str) -> Path:
    """
    Writes code to: <eval_root>/generated_0/<subdir>/<language>/<task_id>_raw.<ext>
    
    Examples:
        split="core_c" -> <eval_root>/generated_0/core/c/<task_id>_raw.c
        split="core_py" -> <eval_root>/generated_0/core/py/<task_id>_raw.py
        split="lang_cpp" -> <eval_root>/generated_0/lang/cpp/<task_id>_raw.cpp
    
    Returns the file path written.
    """
    subdir, language = parse_split(split)
    extension = get_file_extension(language)
    
    out_dir = eval_root / "generated_0" / subdir / language
    out_dir.mkdir(parents=True, exist_ok=True)
    
    raw_path = out_dir / f"{task_id}_raw{extension}"
    with raw_path.open("w", encoding="utf-8") as f:
        f.write(code if code.endswith("\n") else code + "\n")
    return raw_path

def main():
    parser = argparse.ArgumentParser(
        description="Convert CWEval JSON results into CWEval eval folder structure"
    )
    parser.add_argument(
        "--parent",
        required=True,
        help="Parent path under results/CWEval/, e.g. 'deepseek-coder-7b/CoT-SFT_RLVR'"
    )
    parser.add_argument(
        "--results-root",
        default="results/CWEval",
        help="Root folder where results live (default: results/CWEval)"
    )
    parser.add_argument(
        "--cweval-root",
        default="utils/CWEval",
        help="Root folder of CWEval repo (default: utils/CWEval)"
    )
    parser.add_argument(
        "--name",
        default=None,
        help="Optional explicit eval folder name; if omitted, derived from parent"
    )
    args = parser.parse_args()

    # Input JSON path
    json_path = Path(args.results_root) / args.parent / "CWEval_Results.json"
    if not json_path.exists():
        raise FileNotFoundError(f"Could not find results JSON at: {json_path}")

    # Output eval path
    eval_dir_name = args.name or build_eval_dir_name(args.parent)
    eval_root = Path(args.cweval_root) / "evals" / eval_dir_name
    eval_root.mkdir(parents=True, exist_ok=True)

    data = read_json(json_path)
    results = data.get("results", [])
    if not results:
        print(f"No 'results' array found in {json_path}")
        return

    written = 0
    skipped = 0
    lang_counts = {}
    
    for item in results:
        task_id = item.get("id")  # e.g., "cwe_020_0_c"
        split = item.get("split")  # e.g., "core_c"
        out_str = item.get("output_with_tuning", "")

        if not task_id or not split:
            skipped += 1
            continue

        code = extract_code(out_str)
        if not code:
            # If no code present, skip this item
            skipped += 1
            continue

        raw_path = write_code_file(eval_root, task_id, code, split)
        print(f"Wrote: {raw_path}")
        written += 1
        
        # Track language counts
        _, language = parse_split(split)
        lang_counts[language] = lang_counts.get(language, 0) + 1

    print("\nSummary")
    print("-------")
    print(f"Eval root: {eval_root}")
    print(f"Files written: {written}")
    print(f"Items skipped (no code/id/split): {skipped}")
    print(f"\nFiles by language:")
    for lang, count in sorted(lang_counts.items()):
        print(f"  {lang}: {count}")
    print("\nNext steps")
    print("----------")
    print(f"cd {args.cweval_root}")
    print(f"python cweval/evaluate.py pipeline --eval_path evals/{eval_dir_name} --docker True")

if __name__ == "__main__":
    main()

# #!/usr/bin/env python3
# import argparse
# import json
# import os
# import re
# from pathlib import Path
# from typing import Optional

# def read_json(path: Path) -> dict:
#     with path.open("r", encoding="utf-8") as f:
#         return json.load(f)

# CODE_BLOCK_PATTERN_MD = re.compile(
#     r"```(?:python)?\s*(?P<code>[\s\S]*?)\s*```",
#     flags=re.IGNORECASE
# )
# CODE_BLOCK_PATTERN_TAG = re.compile(
#     r"<code>\s*(?P<code>[\s\S]*?)\s*</code>",
#     flags=re.IGNORECASE
# )
# THINK_BLOCK_PATTERN = re.compile(
#     r"<think>[\s\S]*?</think>",
#     flags=re.IGNORECASE
# )

# def extract_code(s: str) -> str:
#     """
#     Extract pure code from an LLM response string.
#     Priority:
#       1) <code> ... </code>
#       2) ```(python)? ... ```
#       3) fallback to raw string (after removing <think>...</think>)
#     """
#     if not isinstance(s, str):
#         return ""

#     # Remove <think> blocks entirely
#     s_wo_think = THINK_BLOCK_PATTERN.sub("", s).strip()

#     # Try <code> ... </code>
#     m = CODE_BLOCK_PATTERN_TAG.search(s_wo_think)
#     if m:
#         return m.group("code").strip()

#     # Try triple-backtick blocks
#     m = CODE_BLOCK_PATTERN_MD.search(s_wo_think)
#     if m:
#         return m.group("code").strip()

#     # Fallback: return remaining text as-is
#     return s_wo_think.strip()

# def build_eval_dir_name(parent_path: str) -> str:
#     """
#     Convert 'deepseek-coder-7b/CoT-SFT_RLVR' -> 'eval_deepseek-coder-7b__CoT-SFT_RLVR'
#     """
#     safe = parent_path.strip("/").replace("/", "__")
#     return f"eval_{safe}"

# def write_py_raw(eval_root: Path, task_id: str, code: str) -> Path:
#     """
#     Writes code to: <eval_root>/generated_0/core/py/<task_id>_raw.py
#     Returns the file path written.
#     """
#     out_dir = eval_root / "generated_0" / "core" / "py"
#     out_dir.mkdir(parents=True, exist_ok=True)
#     raw_path = out_dir / f"{task_id}_raw.py"
#     with raw_path.open("w", encoding="utf-8") as f:
#         f.write(code if code.endswith("\n") else code + "\n")
#     return raw_path

# def main():
#     parser = argparse.ArgumentParser(
#         description="Convert CWEval JSON results into CWEval eval folder structure"
#     )
#     parser.add_argument(
#         "--parent",
#         required=True,
#         help="Parent path under results/CWEval/, e.g. 'deepseek-coder-7b/CoT-SFT_RLVR'"
#     )
#     parser.add_argument(
#         "--results-root",
#         default="results/CWEval",
#         help="Root folder where results live (default: results/CWEval)"
#     )
#     parser.add_argument(
#         "--cweval-root",
#         default="utils/CWEval",
#         help="Root folder of CWEval repo (default: utils/CWEval)"
#     )
#     parser.add_argument(
#         "--name",
#         default=None,
#         help="Optional explicit eval folder name; if omitted, derived from parent"
#     )
#     args = parser.parse_args()

#     # Input JSON path
#     json_path = Path(args.results_root) / args.parent / "CWEval_Results.json"
#     if not json_path.exists():
#         raise FileNotFoundError(f"Could not find results JSON at: {json_path}")

#     # Output eval path
#     eval_dir_name = args.name or build_eval_dir_name(args.parent)
#     eval_root = Path(args.cweval_root) / "evals" / eval_dir_name
#     eval_root.mkdir(parents=True, exist_ok=True)

#     data = read_json(json_path)
#     results = data.get("results", [])
#     if not results:
#         print(f"No 'results' array found in {json_path}")
#         return

#     written = 0
#     skipped = 0
#     for item in results:
#         task_id = item.get("id")  # e.g., "cwe_020_0"
#         out_str = item.get("output_with_tuning", "")

#         if not task_id:
#             skipped += 1
#             continue

#         code = extract_code(out_str)
#         if not code:
#             # If no code present, skip this item
#             skipped += 1
#             continue

#         raw_path = write_py_raw(eval_root, task_id, code)
#         print(f"Wrote: {raw_path}")
#         written += 1

#     print("\nSummary")
#     print("-------")
#     print(f"Eval root: {eval_root}")
#     print(f"Files written: {written}")
#     print(f"Items skipped (no code/id): {skipped}")
#     print("\nNext steps")
#     print("----------")
#     print(f"cd {args.cweval_root}")
#     print(f"python cweval/evaluate.py pipeline --eval_path evals/{eval_dir_name} --docker True")

# if __name__ == "__main__":
#     main()
