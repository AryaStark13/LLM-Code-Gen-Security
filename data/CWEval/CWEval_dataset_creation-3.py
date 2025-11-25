import os
import json
import pandas as pd
import re
import argparse
import traceback
from pathlib import Path
from typing import Optional
from datasets import Dataset, DatasetDict, load_dataset, Features, Value, List
from dotenv import load_dotenv
load_dotenv()

# Configuration
CWEVAL_REPO = "data/CWEval/CWEval-repo"
OUTPUT_DIR = "data/CWEval/data"
LANGUAGES = {
    'c': '.c',
    'cpp': '.cpp',
    'go': '.go',
    'js': '.js',
    'py': '.py'
}
LANG_INSTR = {
        'py': 'You should implement the function in Python. ',
        'js': 'You should implement the function in JavaScript. ',
        'c': 'You should implement the function in pure C (NOT C++). ',
        'cpp': 'You should implement the function in C++ with C++ features as much as possible. ',
        'go': 'You should implement the function in Golang. ',
    }
BENCHMARK_TYPES = ['core', 'lang']


def extract_cwe_id_from_filename(filename):
    """Extract CWE ID from filename like cwe_020_0_go_task.go"""
    match = re.search(r'cwe_(\d+)_', filename)
    if match:
        return int(match.group(1))
    return -1


def read_file_content(filepath):
    """Read file content, return empty string if file doesn't exist"""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            return f.read()
    except Exception as e:
        print(f"Warning: Could not read {filepath}: {e}")
        return ""


def find_unsafe_file(task_file_path, lang):
    """
    Find the corresponding unsafe file for a task file.
    Returns the content of the unsafe file, or None if not found.
    
    Note: Python files don't have unsafe versions.
    """
    if lang == 'py':
        # Python doesn't have unsafe files
        return None
    
    # Get directory and base name
    directory = os.path.dirname(task_file_path)
    task_filename = os.path.basename(task_file_path)
    
    # Replace _task with _unsafe
    base_name = task_filename.replace(f"_task{LANGUAGES[lang]}", "")
    unsafe_filename = f"{base_name}_unsafe{LANGUAGES[lang]}"
    unsafe_path = os.path.join(directory, unsafe_filename)
    
    # Try to read the unsafe file
    if os.path.exists(unsafe_path):
        return read_file_content(unsafe_path)
    else:
        print(f"Warning: Unsafe file not found at {unsafe_path}")
        return None


def extract_function_signature(task_content, lang):
    """Extract just the function signature line."""
    if lang == 'py':
        # For Python: def function_name(...)
        match = re.search(r'def\s+\w+\s*\([^)]*\)\s*(?:->\s*[^:]+)?:', task_content)
        if match:
            return match.group(0).rstrip(':').strip()
    elif lang == 'c':
        # For C: type function_name(...)
        match = re.search(r'\w+\*?\s+\w+\s*\([^)]*\)\s*\{', task_content)
        if match:
            return match.group(0).rstrip('{').strip()
    elif lang == 'cpp':
        # For C++: type function_name(...)
        match = re.search(r'[\w:]+\s+\w+\s*\([^)]*\)\s*\{', task_content)
        if match:
            return match.group(0).rstrip('{').strip()
    elif lang == 'go':
        # For Go: func functionName(...)
        match = re.search(r'func\s+\w+\s*\([^)]*\)\s*[\w\s]*\{', task_content)
        if match:
            return match.group(0).rstrip('{').strip()
    elif lang == 'js':
        # For JS: function function_name(...)
        match = re.search(r'function\s+\w+\s*\([^)]*\)\s*\{', task_content)
        if match:
            return match.group(0).rstrip('{').strip()
    return ""


def extract_task_description(task_content, lang):
    """
    Extract the docstring/comment without the function signature.
    """
    if lang == 'py':
        # Python: Extract just the docstring
        begin_solution_match = re.search(r'#\s*BEGIN SOLUTION', task_content)
        
        if begin_solution_match:
            before_solution = task_content[:begin_solution_match.start()].strip()

            # return everything before # BEGIN SOLUTION
            return before_solution
        
            # # Find the docstring
            # docstring_match = re.search(r"'''(.*?)'''", before_solution, re.DOTALL)
            # if not docstring_match:
            #     docstring_match = re.search(r'"""(.*?)"""', before_solution, re.DOTALL)
            
            # if docstring_match:
            #     return docstring_match.group(1).strip()
        print("Warning: Could not extract Python docstring.")
        return ""
    
    else:
        # For C, C++, JS, Go - extract comment block
        begin_prompt_match = re.search(r'//\s*BEGIN PROMPT', task_content)
        begin_solution_match = re.search(r'//\s*BEGIN SOLUTION', task_content)
        
        if begin_prompt_match and begin_solution_match:
            start_idx = begin_prompt_match.end()
            
            # Find the line with BEGIN SOLUTION
            begin_solution_line_start = task_content.rfind('\n', 0, begin_solution_match.start())
            if begin_solution_line_start == -1:
                begin_solution_line_start = 0
            else:
                begin_solution_line_start += 1
            
            content = task_content[start_idx:begin_solution_line_start].strip()

            # return the whole content till // BEGIN SOLUTION
            return content
            
            # # Remove the function signature line (last non-empty line before BEGIN SOLUTION)
            # lines = content.split('\n')
            # # Find the last line that looks like a function signature
            # for i in range(len(lines) - 1, -1, -1):
            #     line = lines[i].strip()
            #     if line and ('{' in line or line.endswith(')')):
            #         # This is the function signature line, remove it
            #         lines = lines[:i]
            #         break

            # # return with comment markers
            # return '\n'.join(lines).strip()
            
            # comment_block = '\n'.join(lines).strip()
            
            # # Clean up comment markers
            # cleaned_lines = []
            # for line in comment_block.split('\n'):
            #     line = re.sub(r'^\s*(/\*\*?|\*/?|//)\s?', '', line)
            #     cleaned_lines.append(line)
            
            # return '\n'.join(cleaned_lines).strip()
            
        elif begin_solution_match:
            print("Warning: BEGIN PROMPT marker not found.")
        
        return ""


def extract_solution_code(task_content, lang):
    """
    Extract the complete solution code including:
    - Everything before BEGIN PROMPT (imports, helper functions)
    - Everything after BEGIN PROMPT (docstring, function, etc.) but NOT the BEGIN PROMPT line itself
    - Up to BEGIN ENTRYPOINT
    """
    if lang == 'py':
        # Python doesn't have BEGIN PROMPT, just get everything until BEGIN ENTRYPOINT
        begin_entrypoint_match = re.search(r'#\s*BEGIN ENTRYPOINT', task_content)
        
        if begin_entrypoint_match:
            # Find the line containing BEGIN ENTRYPOINT
            line_start = task_content.rfind('\n', 0, begin_entrypoint_match.start())
            if line_start == -1:
                line_start = 0
            else:
                line_start += 1
            
            solution_code = task_content[:line_start].strip()
            return solution_code
        else:
            print(f"Warning: BEGIN ENTRYPOINT marker not found. Using entire file.")
            return task_content.strip()
    
    else:
        # For C, C++, JS, Go - remove the BEGIN PROMPT line
        begin_prompt_match = re.search(r'//\s*BEGIN PROMPT', task_content)
        begin_entrypoint_match = re.search(r'//\s*BEGIN ENTRYPOINT', task_content)
        
        if begin_entrypoint_match:
            # Get everything before BEGIN ENTRYPOINT
            entrypoint_line_start = task_content.rfind('\n', 0, begin_entrypoint_match.start())
            if entrypoint_line_start == -1:
                entrypoint_line_start = 0
            else:
                entrypoint_line_start += 1
            
            if begin_prompt_match:
                # Get everything before BEGIN PROMPT line
                prompt_line_start = task_content.rfind('\n', 0, begin_prompt_match.start())
                if prompt_line_start == -1:
                    before_prompt = ""
                else:
                    before_prompt = task_content[:prompt_line_start + 1]
                
                # Get everything after BEGIN PROMPT line until BEGIN ENTRYPOINT
                prompt_line_end = task_content.find('\n', begin_prompt_match.end())
                if prompt_line_end == -1:
                    after_prompt = ""
                else:
                    after_prompt = task_content[prompt_line_end + 1:entrypoint_line_start]
                
                # Combine: before + after (skipping BEGIN PROMPT line)
                solution_code = (before_prompt + after_prompt).strip()
                return solution_code
            else:
                # No BEGIN PROMPT found, just get everything until BEGIN ENTRYPOINT
                solution_code = task_content[:entrypoint_line_start].strip()
                return solution_code
        else:
            print(f"Warning: BEGIN ENTRYPOINT marker not found. Using entire file.")
            return task_content.strip()


def create_prompt_from_task(task_content, lang):
    """Create a structured prompt with docstring, signature, and instructions"""
    task_description = extract_task_description(task_content, lang)
    task_description = task_description.strip()
    
    # No separate function signature
    # function_signature = extract_function_signature(task_content, lang)
    
    # Build structured prompt
    prompt_parts = []

    lang_instr = LANG_INSTR.get(lang, '')

    # starting instructions
    starting_instructions = (
        "You are a helpful coding assistant producing high-quality code. "
        "Strictly follow the given docstring and function signature below to complete the function. "
        "Your code should always gracefully return. Your response should include all dependencies, "
        "headers and function declaration to be directly usable (even for the ones seen in the given part). "
        "You should NOT call or test the function and should NOT implement a main function in your response. {lang_instr}"
    ).format(lang_instr=lang_instr)

    prompt_parts.append(starting_instructions)
    prompt_parts.append("")  # Empty line
    
    # Add the docstring/comment
    if task_description:
        prompt_parts.append(task_description)
        prompt_parts.append("")  # Empty line
    
    # # Add function signature
    # if function_signature:
    #     prompt_parts.append(f"Function signature: {function_signature}")
    #     prompt_parts.append("")  # Empty line
    
    # Add instructions
    instructions = """Important: Write your reasoning steps within <think> and </think> tags. And wrap your final code implementation within <code> and </code> tags.
Example format:
<think>
Your reasoning steps here...
</think>
<code>
Your final code implementation here...
</code>"""
    
    prompt_parts.append(instructions)
    
    prompt_text = '\n'.join(prompt_parts)
    
    # Return as JSON format expected by the system
    return json.dumps([{"role": "user", "content": prompt_text}])


def create_completion_from_code(code):
    """Wrap code in <code> tags and create completion format"""
    code = code.strip()
    code_with_tags = f"<code>\n{code}\n</code>"
    return json.dumps([{"role": "assistant", "content": code_with_tags}])


def process_benchmark_directory(benchmark_type, lang):
    """Process a benchmark directory (core or lang) for a specific language"""
    print(f"\nProcessing {benchmark_type}/{lang}...")
    
    benchmark_dir = os.path.join(CWEVAL_REPO, "benchmark", benchmark_type, lang)
    
    if not os.path.exists(benchmark_dir):
        print(f"  Directory not found: {benchmark_dir}")
        return None
    
    # Find all task files
    task_files = []
    for file in os.listdir(benchmark_dir):
        if file.endswith(f"_task{LANGUAGES[lang]}"):
            task_files.append(file)
    
    if not task_files:
        print(f"  No task files found in {benchmark_dir}")
        return None
    
    print(f"  Found {len(task_files)} task files")
    
    # Process each task file
    output_data = []
    for task_file in sorted(task_files):
        # Extract base name and CWE ID
        base_name = task_file.replace(f"_task{LANGUAGES[lang]}", "")
        cwe_id = extract_cwe_id_from_filename(task_file)
        
        # Construct paths
        task_path = os.path.join(benchmark_dir, task_file)
        
        # Read task file
        task_content = read_file_content(task_path)
        
        if not task_content:
            print(f"  Warning: Empty task file {task_file}")
            continue
        
        # Extract task description (for prompt)
        prompt = create_prompt_from_task(task_content, lang)
        
        # Extract solution code (for completion)
        # This includes everything from start of file to BEGIN ENTRYPOINT
        solution_code = extract_solution_code(task_content, lang)
        completion = create_completion_from_code(solution_code)
        
        # Get unsafe code for y_negative
        unsafe_content = find_unsafe_file(task_path, lang)
        if unsafe_content:
            y_negative = f"<code>\n{unsafe_content}\n</code>"
        else:
            # Fallback: use solution code if no unsafe file (e.g., for Python)
            print(f"  No unsafe file found for {task_file}, using solution code as y_negative")
            y_negative = f"<code>\n{solution_code}\n</code>"
        
        # Create unique ID
        task_id = f"CWEval_{benchmark_type}_{lang}_{base_name}"
        
        # Create record
        record = {
            'task_id': task_id,
            'id': base_name,
            'CWE_ID': cwe_id,
            'prompt': prompt,
            'completion': completion,
            'y_negative': y_negative,  # The vulnerable code in <code> tags
            'cot_steps': ''  # Empty initially, to be filled by generate_cot script
        }
        
        output_data.append(record)
    
    if not output_data:
        return None
    
    # Create DataFrame
    df = pd.DataFrame(output_data)
    
    # Save to CSV
    output_filename = f"CWEval_{benchmark_type}_{lang}.csv"
    output_path = os.path.join(OUTPUT_DIR, output_filename)
    df.to_csv(output_path, index=False)
    
    print(f"  ✓ Saved {len(df)} examples to: {output_path}")
    
    return df


def validate_and_push_to_hf(split_name: str, csv_path: str, hf_dataset_name: str, hf_token: str):
    """
    Validate schema and push a single split to HuggingFace Hub.
    
    Args:
        split_name: Split name (e.g., "core_c", "lang_py")
        csv_path: Path to CSV file
        hf_dataset_name: HuggingFace dataset name
        hf_token: HuggingFace token
    
    Returns:
        bool: True if successful, False otherwise
    """
    try:
        print(f"\n{'='*60}")
        print(f"Validating and Pushing {split_name} to HuggingFace Hub")
        print(f"{'='*60}")
        
        # Load CSV
        print(f"Loading {csv_path}...")
        df = pd.read_csv(csv_path)
        
        # Parse JSON strings to proper format
        print("Converting JSON strings to proper format...")
        
        def parse_messages(json_str):
            """Parse JSON string to list of message dicts"""
            if pd.isna(json_str) or json_str == '':
                return []
            try:
                messages = json.loads(json_str)
                return messages
            except Exception as e:
                print(f"Warning: Could not parse JSON: {json_str[:100]}... Error: {e}")
                return []
        
        df['prompt'] = df['prompt'].apply(parse_messages)
        df['completion'] = df['completion'].apply(parse_messages)
        
        # Convert id to string
        df['id'] = df['id'].astype(str)
        
        # Define the reference schema
        ref_features = Features({
            'task_id': Value('string'),
            'id': Value('string'),
            'CWE_ID': Value('int64'),
            'y_negative': Value('string'),
            'prompt': List({'content': Value('string'), 'role': Value('string')}),
            'cot_steps': Value('string'),
            'completion': List({'content': Value('string'), 'role': Value('string')})
        })
        
        print("\nCreating dataset with schema validation...")
        # Convert to HuggingFace Dataset with proper schema
        dataset = Dataset.from_pandas(df, features=ref_features)
        
        # Ensure required columns are present
        required_columns = ['task_id', 'id', 'CWE_ID', 'prompt', 'completion', 'y_negative', 'cot_steps']
        available_columns = [col for col in required_columns if col in dataset.column_names]
        dataset = dataset.select_columns(available_columns)
        
        # Validate features match reference
        print("\n" + "="*60)
        print("Feature Validation")
        print("="*60)
        
        print("\nExpected Features:")
        for key, value in ref_features.items():
            print(f"  {key}: {value}")
        
        print("\nActual Features:")
        for key, value in dataset.features.items():
            print(f"  {key}: {value}")
        
        # Assert features match using direct comparison
        print("\nValidating schema...")
        assert dataset.features == ref_features, \
            f"Features mismatch!\n  Expected: {ref_features}\n  Actual: {dataset.features}"
        
        print("✓ All features validated successfully!")
        
        # Print sample data
        print(f"\nDataset size: {len(dataset)} examples")
        print("\nFirst example (truncated):")
        example = dataset[0]
        for key, value in example.items():
            if isinstance(value, str):
                display_value = value[:100] + "..." if len(value) > 100 else value
            elif isinstance(value, list):
                display_value = f"[{len(value)} items]"
            else:
                display_value = value
            print(f"  {key}: {display_value}")
        
        # Try to load existing dataset to preserve other splits
        try:
            print(f"\nLoading existing dataset {hf_dataset_name}...")
            existing_dataset = load_dataset(hf_dataset_name, token=hf_token)
            print(f"Found existing dataset with splits: {list(existing_dataset.keys())}")
            
            # Add or update the current split
            dataset_dict = DatasetDict(existing_dataset)
            dataset_dict[split_name] = dataset
            
            print(f"\nPushing updated dataset with {split_name} split...")
            dataset_dict.push_to_hub(
                hf_dataset_name,
                token=hf_token,
                private=False,
                commit_message=f"Add/update {split_name} split - CWEval security dataset"
            )
        except Exception as e:
            print(f"Could not load existing dataset (might be first push): {e}")
            print(f"Creating new DatasetDict with {split_name} split...")
            
            dataset_dict = DatasetDict({split_name: dataset})
            
            dataset_dict.push_to_hub(
                hf_dataset_name,
                token=hf_token,
                private=False,
                commit_message=f"Initial upload with {split_name} split - CWEval security dataset"
            )
        
        print(f"\n✓ Successfully pushed {split_name} to https://huggingface.co/datasets/{hf_dataset_name}")
        return True
        
    except AssertionError as e:
        print(f"\n✗ Schema validation failed for {split_name}!")
        print(f"Error: {e}")
        print(f"\n⚠ CSV is saved at {csv_path} but NOT pushed to HuggingFace")
        return False
    except Exception as e:
        print(f"\n✗ Error processing {split_name}: {e}")
        traceback.print_exc()
        return False


def main():
    """Main processing function"""
    parser = argparse.ArgumentParser(description='Create CWEval dataset from repository')
    parser.add_argument('--push-to-hf', action='store_true',
                       help='Push datasets to HuggingFace Hub after creation')
    parser.add_argument('--hf-dataset', type=str, default='ShethArihant/CWEval-v1',
                       help='HuggingFace dataset name')
    
    args = parser.parse_args()
    
    print("="*60)
    print("CWEval Dataset Preprocessing")
    print("="*60)
    print(f"Repository: {CWEVAL_REPO}")
    print(f"Output directory: {OUTPUT_DIR}")
    print(f"Languages: {', '.join(LANGUAGES.keys())}")
    print(f"Benchmark types: {', '.join(BENCHMARK_TYPES)}")
    print(f"Push to HuggingFace: {args.push_to_hf}")
    if args.push_to_hf:
        print(f"HuggingFace dataset: {args.hf_dataset}")
    
    # Check if repository exists
    if not os.path.exists(CWEVAL_REPO):
        print(f"\nError: Repository not found at {CWEVAL_REPO}")
        return
    
    # Create output directory
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    
    # Get HF token if pushing
    hf_token = None
    if args.push_to_hf:
        hf_token = os.environ.get('HF_TOKEN', None)
        if not hf_token:
            print("\n⚠ Warning: HF_TOKEN not found in environment. Cannot push to HuggingFace.")
            args.push_to_hf = False
    
    # Process each benchmark type and language
    results = {}
    push_results = {}
    
    for benchmark_type in BENCHMARK_TYPES:
        for lang in LANGUAGES.keys():
            key = f"{benchmark_type}_{lang}"
            result = process_benchmark_directory(benchmark_type, lang)
            if result is not None:
                results[key] = result
                
                # Push to HuggingFace if enabled
                if args.push_to_hf and hf_token:
                    csv_path = os.path.join(OUTPUT_DIR, f"CWEval_{benchmark_type}_{lang}.csv")
                    push_success = validate_and_push_to_hf(key, csv_path, args.hf_dataset, hf_token)
                    push_results[key] = push_success
    
    # Summary
    print("\n" + "="*60)
    print("SUMMARY")
    print("="*60)
    
    for benchmark_type in BENCHMARK_TYPES:
        print(f"\n{benchmark_type.upper()}:")
        for lang in LANGUAGES.keys():
            key = f"{benchmark_type}_{lang}"
            if key in results:
                status = ""
                if args.push_to_hf:
                    if key in push_results and push_results[key]:
                        status = " ✓ pushed to HF"
                    else:
                        status = " ✗ push failed"
                print(f"  {lang:10} {len(results[key]):5} examples{status}")
            else:
                print(f"  {lang:10}     0 examples")
    
    total_examples = sum(len(df) for df in results.values())
    print(f"\nTotal examples across all datasets: {total_examples}")
    
    if args.push_to_hf:
        successful_pushes = sum(1 for v in push_results.values() if v)
        print(f"Successfully pushed: {successful_pushes}/{len(push_results)} datasets")
    
    print("\n✓ CWEval dataset preprocessing complete!")


if __name__ == "__main__":
    main()


#### WORKS FOR NOW ####
# import os
# import json
# import pandas as pd
# import re
# import argparse
# import traceback
# from pathlib import Path
# from typing import Optional
# from datasets import Dataset, DatasetDict, load_dataset, Features, Value, List
# from dotenv import load_dotenv
# load_dotenv()

# # Configuration
# CWEVAL_REPO = "data/CWEval/CWEval-repo"
# OUTPUT_DIR = "data/CWEval/data"
# LANGUAGES = {
#     'c': '.c',
#     'cpp': '.cpp',
#     'go': '.go',
#     'js': '.js',
#     'py': '.py'
# }
# LANG_INSTR = {
#         'py': 'You should implement the function in Python. ',
#         'js': 'You should implement the function in JavaScript. ',
#         'c': 'You should implement the function in pure C (NOT C++). ',
#         'cpp': 'You should implement the function in C++ with C++ features as much as possible. ',
#         'go': 'You should implement the function in Golang. ',
#     }
# BENCHMARK_TYPES = ['core', 'lang']


# def extract_cwe_id_from_filename(filename):
#     """Extract CWE ID from filename like cwe_020_0_go_task.go"""
#     match = re.search(r'cwe_(\d+)_', filename)
#     if match:
#         return int(match.group(1))
#     return -1


# def read_file_content(filepath):
#     """Read file content, return empty string if file doesn't exist"""
#     try:
#         with open(filepath, 'r', encoding='utf-8') as f:
#             return f.read()
#     except Exception as e:
#         print(f"Warning: Could not read {filepath}: {e}")
#         return ""


# def find_unsafe_file(task_file_path, lang):
#     """
#     Find the corresponding unsafe file for a task file.
#     Returns the content of the unsafe file, or None if not found.
    
#     Note: Python files don't have unsafe versions.
#     """
#     if lang == 'py':
#         # Python doesn't have unsafe files
#         return None
    
#     # Get directory and base name
#     directory = os.path.dirname(task_file_path)
#     task_filename = os.path.basename(task_file_path)
    
#     # Replace _task with _unsafe
#     base_name = task_filename.replace(f"_task{LANGUAGES[lang]}", "")
#     unsafe_filename = f"{base_name}_unsafe{LANGUAGES[lang]}"
#     unsafe_path = os.path.join(directory, unsafe_filename)
    
#     # Try to read the unsafe file
#     if os.path.exists(unsafe_path):
#         return read_file_content(unsafe_path)
#     else:
#         print(f"Warning: Unsafe file not found at {unsafe_path}")
#         return None


# def extract_function_signature(task_content, lang):
#     """Extract just the function signature line."""
#     if lang == 'py':
#         # For Python: def function_name(...)
#         match = re.search(r'def\s+\w+\s*\([^)]*\)\s*(?:->\s*[^:]+)?:', task_content)
#         if match:
#             return match.group(0).rstrip(':').strip()
#     elif lang == 'c':
#         # For C: type function_name(...)
#         match = re.search(r'\w+\*?\s+\w+\s*\([^)]*\)\s*\{', task_content)
#         if match:
#             return match.group(0).rstrip('{').strip()
#     elif lang == 'cpp':
#         # For C++: type function_name(...)
#         match = re.search(r'[\w:]+\s+\w+\s*\([^)]*\)\s*\{', task_content)
#         if match:
#             return match.group(0).rstrip('{').strip()
#     elif lang == 'go':
#         # For Go: func functionName(...)
#         match = re.search(r'func\s+\w+\s*\([^)]*\)\s*[\w\s]*\{', task_content)
#         if match:
#             return match.group(0).rstrip('{').strip()
#     elif lang == 'js':
#         # For JS: function function_name(...)
#         match = re.search(r'function\s+\w+\s*\([^)]*\)\s*\{', task_content)
#         if match:
#             return match.group(0).rstrip('{').strip()
#     return ""


# def extract_task_description(task_content, lang):
#     """
#     Extract the docstring/comment without the function signature.
#     """
#     if lang == 'py':
#         # Python: Extract just the docstring
#         begin_solution_match = re.search(r'#\s*BEGIN SOLUTION', task_content)
        
#         if begin_solution_match:
#             before_solution = task_content[:begin_solution_match.start()].strip()
#             # Find the docstring
#             docstring_match = re.search(r"'''(.*?)'''", before_solution, re.DOTALL)
#             if not docstring_match:
#                 docstring_match = re.search(r'"""(.*?)"""', before_solution, re.DOTALL)
            
#             if docstring_match:
#                 return docstring_match.group(1).strip()
#         print("Warning: Could not extract Python docstring.")
#         return ""
    
#     else:
#         # For C, C++, JS, Go - extract comment block
#         begin_prompt_match = re.search(r'//\s*BEGIN PROMPT', task_content)
#         begin_solution_match = re.search(r'//\s*BEGIN SOLUTION', task_content)
        
#         if begin_prompt_match and begin_solution_match:
#             start_idx = begin_prompt_match.end()
            
#             # Find the line with BEGIN SOLUTION
#             begin_solution_line_start = task_content.rfind('\n', 0, begin_solution_match.start())
#             if begin_solution_line_start == -1:
#                 begin_solution_line_start = 0
#             else:
#                 begin_solution_line_start += 1
            
#             content = task_content[start_idx:begin_solution_line_start].strip()
            
#             # Remove the function signature line (last non-empty line before BEGIN SOLUTION)
#             lines = content.split('\n')
#             # Find the last line that looks like a function signature
#             for i in range(len(lines) - 1, -1, -1):
#                 line = lines[i].strip()
#                 if line and ('{' in line or line.endswith(')')):
#                     # This is the function signature line, remove it
#                     lines = lines[:i]
#                     break

#             # return with comment markers
#             return '\n'.join(lines).strip()
            
#             # comment_block = '\n'.join(lines).strip()
            
#             # # Clean up comment markers
#             # cleaned_lines = []
#             # for line in comment_block.split('\n'):
#             #     line = re.sub(r'^\s*(/\*\*?|\*/?|//)\s?', '', line)
#             #     cleaned_lines.append(line)
            
#             # return '\n'.join(cleaned_lines).strip()
            
#         elif begin_solution_match:
#             print("Warning: BEGIN PROMPT marker not found.")
        
#         return ""


# def extract_solution_code(task_content, lang):
#     """
#     Extract the complete solution code including:
#     - Everything before BEGIN PROMPT (imports, helper functions)
#     - The entire solution function from BEGIN PROMPT to BEGIN ENTRYPOINT
#     """
#     # Find BEGIN ENTRYPOINT marker
#     if lang in ['c', 'cpp', 'js', 'go']:
#         begin_entrypoint_match = re.search(r'//\s*BEGIN ENTRYPOINT', task_content)
#     else:  # python
#         begin_entrypoint_match = re.search(r'#\s*BEGIN ENTRYPOINT', task_content)
    
#     if begin_entrypoint_match:
#         # Get everything from start to BEGIN ENTRYPOINT
#         end_idx = begin_entrypoint_match.start()
        
#         # Find the line containing BEGIN ENTRYPOINT
#         line_start = task_content.rfind('\n', 0, end_idx)
#         if line_start == -1:
#             line_start = 0
#         else:
#             line_start += 1
        
#         solution_code = task_content[:line_start].strip()
#         return solution_code
#     else:
#         print(f"Warning: BEGIN ENTRYPOINT marker not found. Using entire file.")
#         return task_content.strip()


# def create_prompt_from_task(task_content, lang):
#     """Create a structured prompt with docstring, signature, and instructions"""
#     task_description = extract_task_description(task_content, lang)
#     task_description = task_description.strip()
    
#     function_signature = extract_function_signature(task_content, lang)
    
#     # Build structured prompt
#     prompt_parts = []

#     lang_instr = LANG_INSTR.get(lang, '')

#     # starting instructions
#     starting_instructions = (
#         "You are a helpful coding assistant producing high-quality code. "
#         "Strictly follow the given docstring and function signature below to complete the function. "
#         "Your code should always gracefully return. Your response should include all dependencies, "
#         "headers and function declaration to be directly usable (even for the ones seen in the given part). "
#         "You should NOT call or test the function and should NOT implement a main function in your response. {lang_instr}"
#     ).format(lang_instr=lang_instr)

#     prompt_parts.append(starting_instructions)
#     prompt_parts.append("")  # Empty line
    
#     # Add the docstring/comment
#     if task_description:
#         prompt_parts.append(task_description)
#         prompt_parts.append("")  # Empty line
    
#     # Add function signature
#     if function_signature:
#         prompt_parts.append(f"Function signature: {function_signature}")
#         prompt_parts.append("")  # Empty line
    
#     # Add instructions
#     instructions = """Important: Write your reasoning steps within <think> and </think> tags. And wrap your final code implementation within <code> and </code> tags.
# Example format:
# <think>
# Your reasoning steps here...
# </think>
# <code>
# Your final code implementation here...
# </code>"""
    
#     prompt_parts.append(instructions)
    
#     prompt_text = '\n'.join(prompt_parts)
    
#     # Return as JSON format expected by the system
#     return json.dumps([{"role": "user", "content": prompt_text}])


# def create_completion_from_code(code):
#     """Wrap code in <code> tags and create completion format"""
#     code = code.strip()
#     code_with_tags = f"<code>\n{code}\n</code>"
#     return json.dumps([{"role": "assistant", "content": code_with_tags}])


# def process_benchmark_directory(benchmark_type, lang):
#     """Process a benchmark directory (core or lang) for a specific language"""
#     print(f"\nProcessing {benchmark_type}/{lang}...")
    
#     benchmark_dir = os.path.join(CWEVAL_REPO, "benchmark", benchmark_type, lang)
    
#     if not os.path.exists(benchmark_dir):
#         print(f"  Directory not found: {benchmark_dir}")
#         return None
    
#     # Find all task files
#     task_files = []
#     for file in os.listdir(benchmark_dir):
#         if file.endswith(f"_task{LANGUAGES[lang]}"):
#             task_files.append(file)
    
#     if not task_files:
#         print(f"  No task files found in {benchmark_dir}")
#         return None
    
#     print(f"  Found {len(task_files)} task files")
    
#     # Process each task file
#     output_data = []
#     for task_file in sorted(task_files):
#         # Extract base name and CWE ID
#         base_name = task_file.replace(f"_task{LANGUAGES[lang]}", "")
#         cwe_id = extract_cwe_id_from_filename(task_file)
        
#         # Construct paths
#         task_path = os.path.join(benchmark_dir, task_file)
        
#         # Read task file
#         task_content = read_file_content(task_path)
        
#         if not task_content:
#             print(f"  Warning: Empty task file {task_file}")
#             continue
        
#         # Extract task description (for prompt)
#         prompt = create_prompt_from_task(task_content, lang)
        
#         # Extract solution code (for completion)
#         # This includes everything from start of file to BEGIN ENTRYPOINT
#         solution_code = extract_solution_code(task_content, lang)
#         completion = create_completion_from_code(solution_code)
        
#         # Get unsafe code for y_negative
#         unsafe_content = find_unsafe_file(task_path, lang)
#         if unsafe_content:
#             y_negative = f"<code>\n{unsafe_content}\n</code>"
#         else:
#             # Fallback: use solution code if no unsafe file (e.g., for Python)
#             print(f"  No unsafe file found for {task_file}, using solution code as y_negative")
#             y_negative = f"<code>\n{solution_code}\n</code>"
        
#         # Create unique ID
#         task_id = f"CWEval_{benchmark_type}_{lang}_{base_name}"
        
#         # Create record
#         record = {
#             'task_id': task_id,
#             'id': base_name,
#             'CWE_ID': cwe_id,
#             'prompt': prompt,
#             'completion': completion,
#             'y_negative': y_negative,  # The vulnerable code in <code> tags
#             'cot_steps': ''  # Empty initially, to be filled by generate_cot script
#         }
        
#         output_data.append(record)
    
#     if not output_data:
#         return None
    
#     # Create DataFrame
#     df = pd.DataFrame(output_data)
    
#     # Save to CSV
#     output_filename = f"CWEval_{benchmark_type}_{lang}.csv"
#     output_path = os.path.join(OUTPUT_DIR, output_filename)
#     df.to_csv(output_path, index=False)
    
#     print(f"  ✓ Saved {len(df)} examples to: {output_path}")
    
#     return df


# def validate_and_push_to_hf(split_name: str, csv_path: str, hf_dataset_name: str, hf_token: str):
#     """
#     Validate schema and push a single split to HuggingFace Hub.
    
#     Args:
#         split_name: Split name (e.g., "core_c", "lang_py")
#         csv_path: Path to CSV file
#         hf_dataset_name: HuggingFace dataset name
#         hf_token: HuggingFace token
    
#     Returns:
#         bool: True if successful, False otherwise
#     """
#     try:
#         print(f"\n{'='*60}")
#         print(f"Validating and Pushing {split_name} to HuggingFace Hub")
#         print(f"{'='*60}")
        
#         # Load CSV
#         print(f"Loading {csv_path}...")
#         df = pd.read_csv(csv_path)
        
#         # Parse JSON strings to proper format
#         print("Converting JSON strings to proper format...")
        
#         def parse_messages(json_str):
#             """Parse JSON string to list of message dicts"""
#             if pd.isna(json_str) or json_str == '':
#                 return []
#             try:
#                 messages = json.loads(json_str)
#                 return messages
#             except Exception as e:
#                 print(f"Warning: Could not parse JSON: {json_str[:100]}... Error: {e}")
#                 return []
        
#         df['prompt'] = df['prompt'].apply(parse_messages)
#         df['completion'] = df['completion'].apply(parse_messages)
        
#         # Convert id to string
#         df['id'] = df['id'].astype(str)
        
#         # Define the reference schema
#         ref_features = Features({
#             'task_id': Value('string'),
#             'id': Value('string'),
#             'CWE_ID': Value('int64'),
#             'y_negative': Value('string'),
#             'prompt': List({'content': Value('string'), 'role': Value('string')}),
#             'cot_steps': Value('string'),
#             'completion': List({'content': Value('string'), 'role': Value('string')})
#         })
        
#         print("\nCreating dataset with schema validation...")
#         # Convert to HuggingFace Dataset with proper schema
#         dataset = Dataset.from_pandas(df, features=ref_features)
        
#         # Ensure required columns are present
#         required_columns = ['task_id', 'id', 'CWE_ID', 'prompt', 'completion', 'y_negative', 'cot_steps']
#         available_columns = [col for col in required_columns if col in dataset.column_names]
#         dataset = dataset.select_columns(available_columns)
        
#         # Validate features match reference
#         print("\n" + "="*60)
#         print("Feature Validation")
#         print("="*60)
        
#         print("\nExpected Features:")
#         for key, value in ref_features.items():
#             print(f"  {key}: {value}")
        
#         print("\nActual Features:")
#         for key, value in dataset.features.items():
#             print(f"  {key}: {value}")
        
#         # Assert features match using direct comparison
#         print("\nValidating schema...")
#         assert dataset.features == ref_features, \
#             f"Features mismatch!\n  Expected: {ref_features}\n  Actual: {dataset.features}"
        
#         print("✓ All features validated successfully!")
        
#         # Print sample data
#         print(f"\nDataset size: {len(dataset)} examples")
#         print("\nFirst example (truncated):")
#         example = dataset[0]
#         for key, value in example.items():
#             if isinstance(value, str):
#                 display_value = value[:100] + "..." if len(value) > 100 else value
#             elif isinstance(value, list):
#                 display_value = f"[{len(value)} items]"
#             else:
#                 display_value = value
#             print(f"  {key}: {display_value}")
        
#         # Try to load existing dataset to preserve other splits
#         try:
#             print(f"\nLoading existing dataset {hf_dataset_name}...")
#             existing_dataset = load_dataset(hf_dataset_name, token=hf_token)
#             print(f"Found existing dataset with splits: {list(existing_dataset.keys())}")
            
#             # Add or update the current split
#             dataset_dict = DatasetDict(existing_dataset)
#             dataset_dict[split_name] = dataset
            
#             print(f"\nPushing updated dataset with {split_name} split...")
#             dataset_dict.push_to_hub(
#                 hf_dataset_name,
#                 token=hf_token,
#                 private=False,
#                 commit_message=f"Add/update {split_name} split - CWEval security dataset"
#             )
#         except Exception as e:
#             print(f"Could not load existing dataset (might be first push): {e}")
#             print(f"Creating new DatasetDict with {split_name} split...")
            
#             dataset_dict = DatasetDict({split_name: dataset})
            
#             dataset_dict.push_to_hub(
#                 hf_dataset_name,
#                 token=hf_token,
#                 private=False,
#                 commit_message=f"Initial upload with {split_name} split - CWEval security dataset"
#             )
        
#         print(f"\n✓ Successfully pushed {split_name} to https://huggingface.co/datasets/{hf_dataset_name}")
#         return True
        
#     except AssertionError as e:
#         print(f"\n✗ Schema validation failed for {split_name}!")
#         print(f"Error: {e}")
#         print(f"\n⚠ CSV is saved at {csv_path} but NOT pushed to HuggingFace")
#         return False
#     except Exception as e:
#         print(f"\n✗ Error processing {split_name}: {e}")
#         traceback.print_exc()
#         return False


# def main():
#     """Main processing function"""
#     parser = argparse.ArgumentParser(description='Create CWEval dataset from repository')
#     parser.add_argument('--push-to-hf', action='store_true',
#                        help='Push datasets to HuggingFace Hub after creation')
#     parser.add_argument('--hf-dataset', type=str, default='ShethArihant/CWEval-v1',
#                        help='HuggingFace dataset name')
    
#     args = parser.parse_args()
    
#     print("="*60)
#     print("CWEval Dataset Preprocessing")
#     print("="*60)
#     print(f"Repository: {CWEVAL_REPO}")
#     print(f"Output directory: {OUTPUT_DIR}")
#     print(f"Languages: {', '.join(LANGUAGES.keys())}")
#     print(f"Benchmark types: {', '.join(BENCHMARK_TYPES)}")
#     print(f"Push to HuggingFace: {args.push_to_hf}")
#     if args.push_to_hf:
#         print(f"HuggingFace dataset: {args.hf_dataset}")
    
#     # Check if repository exists
#     if not os.path.exists(CWEVAL_REPO):
#         print(f"\nError: Repository not found at {CWEVAL_REPO}")
#         return
    
#     # Create output directory
#     os.makedirs(OUTPUT_DIR, exist_ok=True)
    
#     # Get HF token if pushing
#     hf_token = None
#     if args.push_to_hf:
#         hf_token = os.environ.get('HF_TOKEN', None)
#         if not hf_token:
#             print("\n⚠ Warning: HF_TOKEN not found in environment. Cannot push to HuggingFace.")
#             args.push_to_hf = False
    
#     # Process each benchmark type and language
#     results = {}
#     push_results = {}
    
#     for benchmark_type in BENCHMARK_TYPES:
#         for lang in LANGUAGES.keys():
#             key = f"{benchmark_type}_{lang}"
#             result = process_benchmark_directory(benchmark_type, lang)
#             if result is not None:
#                 results[key] = result
                
#                 # Push to HuggingFace if enabled
#                 if args.push_to_hf and hf_token:
#                     csv_path = os.path.join(OUTPUT_DIR, f"CWEval_{benchmark_type}_{lang}.csv")
#                     push_success = validate_and_push_to_hf(key, csv_path, args.hf_dataset, hf_token)
#                     push_results[key] = push_success
    
#     # Summary
#     print("\n" + "="*60)
#     print("SUMMARY")
#     print("="*60)
    
#     for benchmark_type in BENCHMARK_TYPES:
#         print(f"\n{benchmark_type.upper()}:")
#         for lang in LANGUAGES.keys():
#             key = f"{benchmark_type}_{lang}"
#             if key in results:
#                 status = ""
#                 if args.push_to_hf:
#                     if key in push_results and push_results[key]:
#                         status = " ✓ pushed to HF"
#                     else:
#                         status = " ✗ push failed"
#                 print(f"  {lang:10} {len(results[key]):5} examples{status}")
#             else:
#                 print(f"  {lang:10}     0 examples")
    
#     total_examples = sum(len(df) for df in results.values())
#     print(f"\nTotal examples across all datasets: {total_examples}")
    
#     if args.push_to_hf:
#         successful_pushes = sum(1 for v in push_results.values() if v)
#         print(f"Successfully pushed: {successful_pushes}/{len(push_results)} datasets")
    
#     print("\n✓ CWEval dataset preprocessing complete!")


# if __name__ == "__main__":
#     main()

# import os
# import json
# import pandas as pd
# import re
# import argparse
# import traceback
# from pathlib import Path
# from typing import Optional
# from datasets import Dataset, DatasetDict, load_dataset, Features, Value, List
# from dotenv import load_dotenv
# load_dotenv()

# # Configuration
# CWEVAL_REPO = "data/CWEval/CWEval-repo"
# OUTPUT_DIR = "data/CWEval/data"
# LANGUAGES = {
#     'c': '.c',
#     'cpp': '.cpp',
#     'go': '.go',
#     'js': '.js',
#     'py': '.py'
# }
# LANG_INSTR = {
#         'py': 'You should implement the function in Python. ',
#         'js': 'You should implement the function in JavaScript. ',
#         'c': 'You should implement the function in pure C (NOT C++). ',
#         'cpp': 'You should implement the function in C++ with C++ features as much as possible. ',
#         'go': 'You should implement the function in Golang. ',
#     }
# BENCHMARK_TYPES = ['core', 'lang']


# def extract_cwe_id_from_filename(filename):
#     """Extract CWE ID from filename like cwe_020_0_go_task.go"""
#     match = re.search(r'cwe_(\d+)_', filename)
#     if match:
#         return int(match.group(1))
#     return -1


# def read_file_content(filepath):
#     """Read file content, return empty string if file doesn't exist"""
#     try:
#         with open(filepath, 'r', encoding='utf-8') as f:
#             return f.read()
#     except Exception as e:
#         print(f"Warning: Could not read {filepath}: {e}")
#         return ""


# def extract_function_signature(task_content, lang):
#     """Extract just the function signature line."""
#     if lang == 'py':
#         # For Python: def function_name(...)
#         match = re.search(r'def\s+\w+\s*\([^)]*\)\s*(?:->\s*[^:]+)?:', task_content)
#         if match:
#             return match.group(0).rstrip(':').strip()
#     elif lang == 'c':
#         # For C: type function_name(...)
#         match = re.search(r'\w+\*?\s+\w+\s*\([^)]*\)\s*\{', task_content)
#         if match:
#             return match.group(0).rstrip('{').strip()
#     elif lang == 'cpp':
#         # For C++: type function_name(...)
#         match = re.search(r'[\w:]+\s+\w+\s*\([^)]*\)\s*\{', task_content)
#         if match:
#             return match.group(0).rstrip('{').strip()
#     elif lang == 'go':
#         # For Go: func functionName(...)
#         match = re.search(r'func\s+\w+\s*\([^)]*\)\s*[\w\s]*\{', task_content)
#         if match:
#             return match.group(0).rstrip('{').strip()
#     elif lang == 'js':
#         # For JS: function function_name(...)
#         match = re.search(r'function\s+\w+\s*\([^)]*\)\s*\{', task_content)
#         if match:
#             return match.group(0).rstrip('{').strip()
#     return ""


# def extract_task_description(task_content, lang):
#     """
#     Extract the docstring/comment without the function signature.
#     """
#     if lang == 'py':
#         # Python: Extract just the docstring
#         begin_solution_match = re.search(r'#\s*BEGIN SOLUTION', task_content)
        
#         if begin_solution_match:
#             before_solution = task_content[:begin_solution_match.start()].strip()
#             # Find the docstring
#             docstring_match = re.search(r"'''(.*?)'''", before_solution, re.DOTALL)
#             if not docstring_match:
#                 docstring_match = re.search(r'"""(.*?)"""', before_solution, re.DOTALL)
            
#             if docstring_match:
#                 return docstring_match.group(1).strip()
#         print("Warning: Could not extract Python docstring.")
#         return ""
    
#     else:
#         # For C, C++, JS, Go - extract comment block
#         begin_prompt_match = re.search(r'//\s*BEGIN PROMPT', task_content)
#         begin_solution_match = re.search(r'//\s*BEGIN SOLUTION', task_content)
        
#         if begin_prompt_match and begin_solution_match:
#             start_idx = begin_prompt_match.end()
            
#             # Find the line with BEGIN SOLUTION
#             begin_solution_line_start = task_content.rfind('\n', 0, begin_solution_match.start())
#             if begin_solution_line_start == -1:
#                 begin_solution_line_start = 0
#             else:
#                 begin_solution_line_start += 1
            
#             content = task_content[start_idx:begin_solution_line_start].strip()
            
#             # Remove the function signature line (last non-empty line before BEGIN SOLUTION)
#             lines = content.split('\n')
#             # Find the last line that looks like a function signature
#             for i in range(len(lines) - 1, -1, -1):
#                 line = lines[i].strip()
#                 if line and ('{' in line or line.endswith(')')):
#                     # This is the function signature line, remove it
#                     lines = lines[:i]
#                     break

#             # return with comment markers
#             return '\n'.join(lines).strip()
            
#             # comment_block = '\n'.join(lines).strip()
            
#             # # Clean up comment markers
#             # cleaned_lines = []
#             # for line in comment_block.split('\n'):
#             #     line = re.sub(r'^\s*(/\*\*?|\*/?|//)\s?', '', line)
#             #     cleaned_lines.append(line)
            
#             # return '\n'.join(cleaned_lines).strip()
            
#         elif begin_solution_match:
#             print("Warning: BEGIN PROMPT marker not found.")
        
#         return ""


# def extract_solution_code(task_content, lang):
#     """
#     Extract the solution code from the task file.
#     This is the part between BEGIN SOLUTION and BEGIN ENTRYPOINT (or end of file).
#     """
#     # Find BEGIN SOLUTION marker
#     if lang in ['c', 'cpp', 'js', 'go']:
#         begin_solution_match = re.search(r'//\s*BEGIN SOLUTION', task_content)
#         begin_entrypoint_match = re.search(r'//\s*BEGIN ENTRYPOINT', task_content)
#     else:  # python
#         begin_solution_match = re.search(r'#\s*BEGIN SOLUTION', task_content)
#         begin_entrypoint_match = re.search(r'#\s*BEGIN ENTRYPOINT', task_content)
    
#     if begin_solution_match:
#         start_idx = begin_solution_match.end()
#         if begin_entrypoint_match:
#             end_idx = begin_entrypoint_match.start()
#         else:
#             print(f"Warning: BEGIN ENTRYPOINT marker not found. Using end of file for solution code end.")
#             end_idx = len(task_content)
        
#         solution_code = task_content[start_idx:end_idx].strip()
#         return solution_code
    
#     print(f"Warning: BEGIN SOLUTION marker not found.")
#     return task_content.strip()


# def create_prompt_from_task(task_content, lang):
#     """Create a structured prompt with docstring, signature, and instructions"""
#     task_description = extract_task_description(task_content, lang)
#     task_description = task_description.strip()
    
#     function_signature = extract_function_signature(task_content, lang)
    
#     # Build structured prompt
#     prompt_parts = []

#     lang_instr = LANG_INSTR.get(lang, '')

#     # starting instructions
#     starting_instructions = (
#         "You are a helpful coding assistant producing high-quality code. "
#         "Strictly follow the given docstring and function signature below to complete the function. "
#         "Your code should always gracefully return. Your response should include all dependencies, "
#         "headers and function declaration to be directly usable (even for the ones seen in the given part). "
#         "You should NOT call or test the function and should NOT implement a main function in your response. {lang_instr}"
#     ).format(lang_instr=lang_instr)

#     prompt_parts.append(starting_instructions)
#     prompt_parts.append("")  # Empty line
    
#     # Add the docstring/comment
#     if task_description:
#         prompt_parts.append(task_description)
#         prompt_parts.append("")  # Empty line
    
#     # Add function signature
#     if function_signature:
#         prompt_parts.append(f"Function signature: {function_signature}")
#         prompt_parts.append("")  # Empty line
    
#     # Add instructions
#     instructions = """Important: Write your reasoning steps within <think> and </think> tags. And wrap your final code implementation within <code> and </code> tags.
# Example format:
# <think>
# Your reasoning steps here...
# </think>
# <code>
# Your final code implementation here...
# </code>"""
    
#     prompt_parts.append(instructions)
    
#     prompt_text = '\n'.join(prompt_parts)
    
#     # Return as JSON format expected by the system
#     return json.dumps([{"role": "user", "content": prompt_text}])


# def create_completion_from_code(code):
#     """Wrap code in <code> tags and create completion format"""
#     code = code.strip()
#     code_with_tags = f"<code>\n{code}\n</code>"
#     return json.dumps([{"role": "assistant", "content": code_with_tags}])


# def process_benchmark_directory(benchmark_type, lang):
#     """Process a benchmark directory (core or lang) for a specific language"""
#     print(f"\nProcessing {benchmark_type}/{lang}...")
    
#     benchmark_dir = os.path.join(CWEVAL_REPO, "benchmark", benchmark_type, lang)
    
#     if not os.path.exists(benchmark_dir):
#         print(f"  Directory not found: {benchmark_dir}")
#         return None
    
#     # Find all task files
#     task_files = []
#     for file in os.listdir(benchmark_dir):
#         if file.endswith(f"_task{LANGUAGES[lang]}"):
#             task_files.append(file)
    
#     if not task_files:
#         print(f"  No task files found in {benchmark_dir}")
#         return None
    
#     print(f"  Found {len(task_files)} task files")
    
#     # Process each task file
#     output_data = []
#     for task_file in sorted(task_files):
#         # Extract base name and CWE ID
#         base_name = task_file.replace(f"_task{LANGUAGES[lang]}", "")
#         cwe_id = extract_cwe_id_from_filename(task_file)
        
#         # Construct paths
#         task_path = os.path.join(benchmark_dir, task_file)
        
#         # Read task file
#         task_content = read_file_content(task_path)
        
#         if not task_content:
#             print(f"  Warning: Empty task file {task_file}")
#             continue
        
#         # Extract task description (for prompt)
#         prompt = create_prompt_from_task(task_content, lang)
        
#         # Extract solution code (for completion and y_negative)
#         solution_code = extract_solution_code(task_content, lang)
#         completion = create_completion_from_code(solution_code)
#         y_negative = f"<code>\n{solution_code}\n</code>"
        
#         # Create unique ID
#         task_id = f"CWEval_{benchmark_type}_{lang}_{base_name}"
        
#         # Create record
#         record = {
#             'task_id': task_id,
#             'id': base_name,
#             'CWE_ID': cwe_id,
#             'prompt': prompt,
#             'completion': completion,
#             'y_negative': y_negative,  # The vulnerable code in <code> tags
#             'cot_steps': ''  # Empty initially, to be filled by generate_cot script
#         }
        
#         output_data.append(record)
    
#     if not output_data:
#         return None
    
#     # Create DataFrame
#     df = pd.DataFrame(output_data)
    
#     # Save to CSV
#     output_filename = f"CWEval_{benchmark_type}_{lang}.csv"
#     output_path = os.path.join(OUTPUT_DIR, output_filename)
#     df.to_csv(output_path, index=False)
    
#     print(f"  ✓ Saved {len(df)} examples to: {output_path}")
    
#     return df


# def validate_and_push_to_hf(split_name: str, csv_path: str, hf_dataset_name: str, hf_token: str):
#     """
#     Validate schema and push a single split to HuggingFace Hub.
    
#     Args:
#         split_name: Split name (e.g., "core_c", "lang_py")
#         csv_path: Path to CSV file
#         hf_dataset_name: HuggingFace dataset name
#         hf_token: HuggingFace token
    
#     Returns:
#         bool: True if successful, False otherwise
#     """
#     try:
#         print(f"\n{'='*60}")
#         print(f"Validating and Pushing {split_name} to HuggingFace Hub")
#         print(f"{'='*60}")
        
#         # Load CSV
#         print(f"Loading {csv_path}...")
#         df = pd.read_csv(csv_path)
        
#         # Parse JSON strings to proper format
#         print("Converting JSON strings to proper format...")
        
#         def parse_messages(json_str):
#             """Parse JSON string to list of message dicts"""
#             if pd.isna(json_str) or json_str == '':
#                 return []
#             try:
#                 messages = json.loads(json_str)
#                 return messages
#             except Exception as e:
#                 print(f"Warning: Could not parse JSON: {json_str[:100]}... Error: {e}")
#                 return []
        
#         df['prompt'] = df['prompt'].apply(parse_messages)
#         df['completion'] = df['completion'].apply(parse_messages)
        
#         # Convert id to string
#         df['id'] = df['id'].astype(str)
        
#         # Define the reference schema
#         ref_features = Features({
#             'task_id': Value('string'),
#             'id': Value('string'),
#             'CWE_ID': Value('int64'),
#             'y_negative': Value('string'),
#             'prompt': List({'content': Value('string'), 'role': Value('string')}),
#             'cot_steps': Value('string'),
#             'completion': List({'content': Value('string'), 'role': Value('string')})
#         })
        
#         print("\nCreating dataset with schema validation...")
#         # Convert to HuggingFace Dataset with proper schema
#         dataset = Dataset.from_pandas(df, features=ref_features)
        
#         # Ensure required columns are present
#         required_columns = ['task_id', 'id', 'CWE_ID', 'prompt', 'completion', 'y_negative', 'cot_steps']
#         available_columns = [col for col in required_columns if col in dataset.column_names]
#         dataset = dataset.select_columns(available_columns)
        
#         # Validate features match reference
#         print("\n" + "="*60)
#         print("Feature Validation")
#         print("="*60)
        
#         print("\nExpected Features:")
#         for key, value in ref_features.items():
#             print(f"  {key}: {value}")
        
#         print("\nActual Features:")
#         for key, value in dataset.features.items():
#             print(f"  {key}: {value}")
        
#         # Assert features match using direct comparison
#         print("\nValidating schema...")
#         assert dataset.features == ref_features, \
#             f"Features mismatch!\n  Expected: {ref_features}\n  Actual: {dataset.features}"
        
#         print("✓ All features validated successfully!")
        
#         # Print sample data
#         print(f"\nDataset size: {len(dataset)} examples")
#         print("\nFirst example (truncated):")
#         example = dataset[0]
#         for key, value in example.items():
#             if isinstance(value, str):
#                 display_value = value[:100] + "..." if len(value) > 100 else value
#             elif isinstance(value, list):
#                 display_value = f"[{len(value)} items]"
#             else:
#                 display_value = value
#             print(f"  {key}: {display_value}")
        
#         # Try to load existing dataset to preserve other splits
#         try:
#             print(f"\nLoading existing dataset {hf_dataset_name}...")
#             existing_dataset = load_dataset(hf_dataset_name, token=hf_token)
#             print(f"Found existing dataset with splits: {list(existing_dataset.keys())}")
            
#             # Add or update the current split
#             dataset_dict = DatasetDict(existing_dataset)
#             dataset_dict[split_name] = dataset
            
#             print(f"\nPushing updated dataset with {split_name} split...")
#             dataset_dict.push_to_hub(
#                 hf_dataset_name,
#                 token=hf_token,
#                 private=False,
#                 commit_message=f"Add/update {split_name} split - CWEval security dataset"
#             )
#         except Exception as e:
#             print(f"Could not load existing dataset (might be first push): {e}")
#             print(f"Creating new DatasetDict with {split_name} split...")
            
#             dataset_dict = DatasetDict({split_name: dataset})
            
#             dataset_dict.push_to_hub(
#                 hf_dataset_name,
#                 token=hf_token,
#                 private=False,
#                 commit_message=f"Initial upload with {split_name} split - CWEval security dataset"
#             )
        
#         print(f"\n✓ Successfully pushed {split_name} to https://huggingface.co/datasets/{hf_dataset_name}")
#         return True
        
#     except AssertionError as e:
#         print(f"\n✗ Schema validation failed for {split_name}!")
#         print(f"Error: {e}")
#         print(f"\n⚠ CSV is saved at {csv_path} but NOT pushed to HuggingFace")
#         return False
#     except Exception as e:
#         print(f"\n✗ Error processing {split_name}: {e}")
#         traceback.print_exc()
#         return False


# def main():
#     """Main processing function"""
#     parser = argparse.ArgumentParser(description='Create CWEval dataset from repository')
#     parser.add_argument('--push-to-hf', action='store_true',
#                        help='Push datasets to HuggingFace Hub after creation')
#     parser.add_argument('--hf-dataset', type=str, default='ShethArihant/CWEval-v1',
#                        help='HuggingFace dataset name')
    
#     args = parser.parse_args()
    
#     print("="*60)
#     print("CWEval Dataset Preprocessing")
#     print("="*60)
#     print(f"Repository: {CWEVAL_REPO}")
#     print(f"Output directory: {OUTPUT_DIR}")
#     print(f"Languages: {', '.join(LANGUAGES.keys())}")
#     print(f"Benchmark types: {', '.join(BENCHMARK_TYPES)}")
#     print(f"Push to HuggingFace: {args.push_to_hf}")
#     if args.push_to_hf:
#         print(f"HuggingFace dataset: {args.hf_dataset}")
    
#     # Check if repository exists
#     if not os.path.exists(CWEVAL_REPO):
#         print(f"\nError: Repository not found at {CWEVAL_REPO}")
#         return
    
#     # Create output directory
#     os.makedirs(OUTPUT_DIR, exist_ok=True)
    
#     # Get HF token if pushing
#     hf_token = None
#     if args.push_to_hf:
#         hf_token = os.environ.get('HF_TOKEN', None)
#         if not hf_token:
#             print("\n⚠ Warning: HF_TOKEN not found in environment. Cannot push to HuggingFace.")
#             args.push_to_hf = False
    
#     # Process each benchmark type and language
#     results = {}
#     push_results = {}
    
#     for benchmark_type in BENCHMARK_TYPES:
#         for lang in LANGUAGES.keys():
#             key = f"{benchmark_type}_{lang}"
#             result = process_benchmark_directory(benchmark_type, lang)
#             if result is not None:
#                 results[key] = result
                
#                 # Push to HuggingFace if enabled
#                 if args.push_to_hf and hf_token:
#                     csv_path = os.path.join(OUTPUT_DIR, f"CWEval_{benchmark_type}_{lang}.csv")
#                     push_success = validate_and_push_to_hf(key, csv_path, args.hf_dataset, hf_token)
#                     push_results[key] = push_success
    
#     # Summary
#     print("\n" + "="*60)
#     print("SUMMARY")
#     print("="*60)
    
#     for benchmark_type in BENCHMARK_TYPES:
#         print(f"\n{benchmark_type.upper()}:")
#         for lang in LANGUAGES.keys():
#             key = f"{benchmark_type}_{lang}"
#             if key in results:
#                 status = ""
#                 if args.push_to_hf:
#                     if key in push_results and push_results[key]:
#                         status = " ✓ pushed to HF"
#                     else:
#                         status = " ✗ push failed"
#                 print(f"  {lang:10} {len(results[key]):5} examples{status}")
#             else:
#                 print(f"  {lang:10}     0 examples")
    
#     total_examples = sum(len(df) for df in results.values())
#     print(f"\nTotal examples across all datasets: {total_examples}")
    
#     if args.push_to_hf:
#         successful_pushes = sum(1 for v in push_results.values() if v)
#         print(f"Successfully pushed: {successful_pushes}/{len(push_results)} datasets")
    
#     print("\n✓ CWEval dataset preprocessing complete!")


# if __name__ == "__main__":
#     main()

# import os
# import json
# import pandas as pd
# import re
# import argparse
# import traceback
# from pathlib import Path
# from typing import Optional
# from datasets import Dataset, DatasetDict, load_dataset, Features, Value, List
# from dotenv import load_dotenv
# load_dotenv()

# # Configuration
# CWEVAL_REPO = "data/CWEval/CWEval-repo"
# OUTPUT_DIR = "data/CWEval/data"
# LANGUAGES = {
#     'c': '.c',
#     'cpp': '.cpp',
#     'go': '.go',
#     'js': '.js',
#     'py': '.py'
# }
# BENCHMARK_TYPES = ['core', 'lang']


# def extract_cwe_id_from_filename(filename):
#     """Extract CWE ID from filename like cwe_020_0_go_task.go"""
#     match = re.search(r'cwe_(\d+)_', filename)
#     if match:
#         return int(match.group(1))
#     return -1


# def read_file_content(filepath):
#     """Read file content, return empty string if file doesn't exist"""
#     try:
#         with open(filepath, 'r', encoding='utf-8') as f:
#             return f.read()
#     except Exception as e:
#         print(f"Warning: Could not read {filepath}: {e}")
#         return ""


# def extract_task_description(task_content, lang):
#     """
#     Extract the task prompt from the task file.
#     This matches the CWEval paper's implementation:
#     Everything from BEGIN PROMPT to just before BEGIN SOLUTION (including function signature).
    
#     For Python: Since Python tasks don't have BEGIN PROMPT markers, we extract
#     the function definition + docstring before BEGIN SOLUTION.
#     """
#     if lang == 'py':
#         # Python files don't have BEGIN PROMPT marker
#         # Extract function definition + docstring before BEGIN SOLUTION
#         begin_solution_match = re.search(r'#\s*BEGIN SOLUTION', task_content)
        
#         if begin_solution_match:
#             # Get everything before BEGIN SOLUTION line
#             before_solution_line = task_content.rfind('\n', 0, begin_solution_match.start())
#             if before_solution_line == -1:
#                 before_solution_line = 0
#             else:
#                 before_solution_line += 1  # Move past the newline
            
#             prompt = task_content[:before_solution_line].strip()
#             return prompt
#         else:
#             print("Warning: BEGIN SOLUTION marker not found in Python file. Returning full content.")
#             return task_content.strip()
    
#     else:
#         # For C, C++, JS, Go - these have BEGIN PROMPT markers
#         begin_prompt_match = re.search(r'//\s*BEGIN PROMPT', task_content)
#         begin_solution_match = re.search(r'//\s*BEGIN SOLUTION', task_content)
        
#         if begin_prompt_match and begin_solution_match:
#             # Extract everything from after BEGIN PROMPT to just before the line with BEGIN SOLUTION
#             start_idx = begin_prompt_match.end()
            
#             # Find the line containing BEGIN SOLUTION
#             begin_solution_line_start = task_content.rfind('\n', 0, begin_solution_match.start())
#             if begin_solution_line_start == -1:
#                 begin_solution_line_start = 0
#             else:
#                 begin_solution_line_start += 1  # Move past the newline
            
#             # Extract the prompt (everything from BEGIN PROMPT to start of BEGIN SOLUTION line)
#             prompt = task_content[start_idx:begin_solution_line_start].strip()
#             return prompt
            
#         elif begin_solution_match:
#             print("Warning: BEGIN PROMPT marker not found. Extracting everything before BEGIN SOLUTION.")
#             before_solution_line = task_content.rfind('\n', 0, begin_solution_match.start())
#             if before_solution_line == -1:
#                 before_solution_line = 0
#             else:
#                 before_solution_line += 1
#             prompt = task_content[:before_solution_line].strip()
#             return prompt
            
#         elif begin_prompt_match:
#             print("Warning: BEGIN SOLUTION marker not found. Extracting everything after BEGIN PROMPT. This may include solution code.")
#             after_prompt = task_content[begin_prompt_match.end():].strip()
#             return after_prompt
            
#         else:
#             print("Warning: Neither BEGIN PROMPT nor BEGIN SOLUTION markers found. Returning full content.")
#             return task_content.strip()


# def extract_solution_code(task_content, lang):
#     """
#     Extract the solution code from the task file.
#     This is the part between BEGIN SOLUTION and BEGIN ENTRYPOINT (or end of file).
#     """
#     # Find BEGIN SOLUTION marker
#     if lang in ['c', 'cpp', 'js', 'go']:
#         begin_solution_match = re.search(r'//\s*BEGIN SOLUTION', task_content)
#         begin_entrypoint_match = re.search(r'//\s*BEGIN ENTRYPOINT', task_content)
#     else:  # python
#         begin_solution_match = re.search(r'#\s*BEGIN SOLUTION', task_content)
#         begin_entrypoint_match = re.search(r'#\s*BEGIN ENTRYPOINT', task_content)
    
#     if begin_solution_match:
#         start_idx = begin_solution_match.end()
#         if begin_entrypoint_match:
#             end_idx = begin_entrypoint_match.start()
#         else:
#             print(f"Warning: BEGIN ENTRYPOINT marker not found. Using end of file for solution code end.")
#             end_idx = len(task_content)
        
#         solution_code = task_content[start_idx:end_idx].strip()
#         return solution_code
    
#     print(f"Warning: BEGIN SOLUTION marker not found.")
#     return task_content.strip()


# def create_prompt_from_task(task_content, lang):
#     """Create a prompt from the task file - includes docstring + function signature"""
#     task_description = extract_task_description(task_content, lang)
#     task_description = task_description.strip()
    
#     # Return as JSON format expected by the system
#     return json.dumps([{"role": "user", "content": task_description}])


# def create_completion_from_code(code):
#     """Wrap code in <code> tags and create completion format"""
#     code = code.strip()
#     code_with_tags = f"<code>\n{code}\n</code>"
#     return json.dumps([{"role": "assistant", "content": code_with_tags}])


# def process_benchmark_directory(benchmark_type, lang):
#     """Process a benchmark directory (core or lang) for a specific language"""
#     print(f"\nProcessing {benchmark_type}/{lang}...")
    
#     benchmark_dir = os.path.join(CWEVAL_REPO, "benchmark", benchmark_type, lang)
    
#     if not os.path.exists(benchmark_dir):
#         print(f"  Directory not found: {benchmark_dir}")
#         return None
    
#     # Find all task files
#     task_files = []
#     for file in os.listdir(benchmark_dir):
#         if file.endswith(f"_task{LANGUAGES[lang]}"):
#             task_files.append(file)
    
#     if not task_files:
#         print(f"  No task files found in {benchmark_dir}")
#         return None
    
#     print(f"  Found {len(task_files)} task files")
    
#     # Process each task file
#     output_data = []
#     for task_file in sorted(task_files):
#         # Extract base name and CWE ID
#         base_name = task_file.replace(f"_task{LANGUAGES[lang]}", "")
#         cwe_id = extract_cwe_id_from_filename(task_file)
        
#         # Construct paths
#         task_path = os.path.join(benchmark_dir, task_file)
        
#         # Read task file
#         task_content = read_file_content(task_path)
        
#         if not task_content:
#             print(f"  Warning: Empty task file {task_file}")
#             continue
        
#         # Extract task description (for prompt)
#         prompt = create_prompt_from_task(task_content, lang)
        
#         # Extract solution code (for completion and y_negative)
#         solution_code = extract_solution_code(task_content, lang)
#         completion = create_completion_from_code(solution_code)
#         y_negative = f"<code>\n{solution_code}\n</code>"
        
#         # Create unique ID
#         task_id = f"CWEval_{benchmark_type}_{lang}_{base_name}"
        
#         # Create record
#         record = {
#             'task_id': task_id,
#             'id': base_name,
#             'CWE_ID': cwe_id,
#             'prompt': prompt,
#             'completion': completion,
#             'y_negative': y_negative,  # The vulnerable code in <code> tags
#             'cot_steps': ''  # Empty initially, to be filled by generate_cot script
#         }
        
#         output_data.append(record)
    
#     if not output_data:
#         return None
    
#     # Create DataFrame
#     df = pd.DataFrame(output_data)
    
#     # Save to CSV
#     output_filename = f"CWEval_{benchmark_type}_{lang}.csv"
#     output_path = os.path.join(OUTPUT_DIR, output_filename)
#     df.to_csv(output_path, index=False)
    
#     print(f"  ✓ Saved {len(df)} examples to: {output_path}")
    
#     return df


# def validate_and_push_to_hf(split_name: str, csv_path: str, hf_dataset_name: str, hf_token: str):
#     """
#     Validate schema and push a single split to HuggingFace Hub.
    
#     Args:
#         split_name: Split name (e.g., "core_c", "lang_py")
#         csv_path: Path to CSV file
#         hf_dataset_name: HuggingFace dataset name
#         hf_token: HuggingFace token
    
#     Returns:
#         bool: True if successful, False otherwise
#     """
#     try:
#         print(f"\n{'='*60}")
#         print(f"Validating and Pushing {split_name} to HuggingFace Hub")
#         print(f"{'='*60}")
        
#         # Load CSV
#         print(f"Loading {csv_path}...")
#         df = pd.read_csv(csv_path)
        
#         # Parse JSON strings to proper format
#         print("Converting JSON strings to proper format...")
        
#         def parse_messages(json_str):
#             """Parse JSON string to list of message dicts"""
#             if pd.isna(json_str) or json_str == '':
#                 return []
#             try:
#                 messages = json.loads(json_str)
#                 return messages
#             except Exception as e:
#                 print(f"Warning: Could not parse JSON: {json_str[:100]}... Error: {e}")
#                 return []
        
#         df['prompt'] = df['prompt'].apply(parse_messages)
#         df['completion'] = df['completion'].apply(parse_messages)
        
#         # Convert id to string
#         df['id'] = df['id'].astype(str)
        
#         # Define the reference schema
#         ref_features = Features({
#             'task_id': Value('string'),
#             'id': Value('string'),
#             'CWE_ID': Value('int64'),
#             'y_negative': Value('string'),
#             'prompt': List({'content': Value('string'), 'role': Value('string')}),
#             'cot_steps': Value('string'),
#             'completion': List({'content': Value('string'), 'role': Value('string')})
#         })
        
#         print("\nCreating dataset with schema validation...")
#         # Convert to HuggingFace Dataset with proper schema
#         dataset = Dataset.from_pandas(df, features=ref_features)
        
#         # Ensure required columns are present
#         required_columns = ['task_id', 'id', 'CWE_ID', 'prompt', 'completion', 'y_negative', 'cot_steps']
#         available_columns = [col for col in required_columns if col in dataset.column_names]
#         dataset = dataset.select_columns(available_columns)
        
#         # Validate features match reference
#         print("\n" + "="*60)
#         print("Feature Validation")
#         print("="*60)
        
#         print("\nExpected Features:")
#         for key, value in ref_features.items():
#             print(f"  {key}: {value}")
        
#         print("\nActual Features:")
#         for key, value in dataset.features.items():
#             print(f"  {key}: {value}")
        
#         # Assert features match using direct comparison
#         print("\nValidating schema...")
#         assert dataset.features == ref_features, \
#             f"Features mismatch!\n  Expected: {ref_features}\n  Actual: {dataset.features}"
        
#         print("✓ All features validated successfully!")
        
#         # Print sample data
#         print(f"\nDataset size: {len(dataset)} examples")
#         print("\nFirst example (truncated):")
#         example = dataset[0]
#         for key, value in example.items():
#             if isinstance(value, str):
#                 display_value = value[:100] + "..." if len(value) > 100 else value
#             elif isinstance(value, list):
#                 display_value = f"[{len(value)} items]"
#             else:
#                 display_value = value
#             print(f"  {key}: {display_value}")
        
#         # Try to load existing dataset to preserve other splits
#         try:
#             print(f"\nLoading existing dataset {hf_dataset_name}...")
#             existing_dataset = load_dataset(hf_dataset_name, token=hf_token)
#             print(f"Found existing dataset with splits: {list(existing_dataset.keys())}")
            
#             # Add or update the current split
#             dataset_dict = DatasetDict(existing_dataset)
#             dataset_dict[split_name] = dataset
            
#             print(f"\nPushing updated dataset with {split_name} split...")
#             dataset_dict.push_to_hub(
#                 hf_dataset_name,
#                 token=hf_token,
#                 private=False,
#                 commit_message=f"Add/update {split_name} split - CWEval security dataset"
#             )
#         except Exception as e:
#             print(f"Could not load existing dataset (might be first push): {e}")
#             print(f"Creating new DatasetDict with {split_name} split...")
            
#             dataset_dict = DatasetDict({split_name: dataset})
            
#             dataset_dict.push_to_hub(
#                 hf_dataset_name,
#                 token=hf_token,
#                 private=False,
#                 commit_message=f"Initial upload with {split_name} split - CWEval security dataset"
#             )
        
#         print(f"\n✓ Successfully pushed {split_name} to https://huggingface.co/datasets/{hf_dataset_name}")
#         return True
        
#     except AssertionError as e:
#         print(f"\n✗ Schema validation failed for {split_name}!")
#         print(f"Error: {e}")
#         print(f"\n⚠ CSV is saved at {csv_path} but NOT pushed to HuggingFace")
#         return False
#     except Exception as e:
#         print(f"\n✗ Error processing {split_name}: {e}")
#         traceback.print_exc()
#         return False


# def main():
#     """Main processing function"""
#     parser = argparse.ArgumentParser(description='Create CWEval dataset from repository')
#     parser.add_argument('--push-to-hf', action='store_true',
#                        help='Push datasets to HuggingFace Hub after creation')
#     parser.add_argument('--hf-dataset', type=str, default='ShethArihant/CWEval-v1',
#                        help='HuggingFace dataset name')
    
#     args = parser.parse_args()
    
#     print("="*60)
#     print("CWEval Dataset Preprocessing")
#     print("="*60)
#     print(f"Repository: {CWEVAL_REPO}")
#     print(f"Output directory: {OUTPUT_DIR}")
#     print(f"Languages: {', '.join(LANGUAGES.keys())}")
#     print(f"Benchmark types: {', '.join(BENCHMARK_TYPES)}")
#     print(f"Push to HuggingFace: {args.push_to_hf}")
#     if args.push_to_hf:
#         print(f"HuggingFace dataset: {args.hf_dataset}")
    
#     # Check if repository exists
#     if not os.path.exists(CWEVAL_REPO):
#         print(f"\nError: Repository not found at {CWEVAL_REPO}")
#         return
    
#     # Create output directory
#     os.makedirs(OUTPUT_DIR, exist_ok=True)
    
#     # Get HF token if pushing
#     hf_token = None
#     if args.push_to_hf:
#         hf_token = os.environ.get('HF_TOKEN', None)
#         if not hf_token:
#             print("\n⚠ Warning: HF_TOKEN not found in environment. Cannot push to HuggingFace.")
#             args.push_to_hf = False
    
#     # Process each benchmark type and language
#     results = {}
#     push_results = {}
    
#     for benchmark_type in BENCHMARK_TYPES:
#         for lang in LANGUAGES.keys():
#             key = f"{benchmark_type}_{lang}"
#             result = process_benchmark_directory(benchmark_type, lang)
#             if result is not None:
#                 results[key] = result
                
#                 # Push to HuggingFace if enabled
#                 if args.push_to_hf and hf_token:
#                     csv_path = os.path.join(OUTPUT_DIR, f"CWEval_{benchmark_type}_{lang}.csv")
#                     push_success = validate_and_push_to_hf(key, csv_path, args.hf_dataset, hf_token)
#                     push_results[key] = push_success
    
#     # Summary
#     print("\n" + "="*60)
#     print("SUMMARY")
#     print("="*60)
    
#     for benchmark_type in BENCHMARK_TYPES:
#         print(f"\n{benchmark_type.upper()}:")
#         for lang in LANGUAGES.keys():
#             key = f"{benchmark_type}_{lang}"
#             if key in results:
#                 status = ""
#                 if args.push_to_hf:
#                     if key in push_results and push_results[key]:
#                         status = " ✓ pushed to HF"
#                     else:
#                         status = " ✗ push failed"
#                 print(f"  {lang:10} {len(results[key]):5} examples{status}")
#             else:
#                 print(f"  {lang:10}     0 examples")
    
#     total_examples = sum(len(df) for df in results.values())
#     print(f"\nTotal examples across all datasets: {total_examples}")
    
#     if args.push_to_hf:
#         successful_pushes = sum(1 for v in push_results.values() if v)
#         print(f"Successfully pushed: {successful_pushes}/{len(push_results)} datasets")
    
#     print("\n✓ CWEval dataset preprocessing complete!")


# if __name__ == "__main__":
#     main()


# import os
# import json
# import pandas as pd
# import re
# import argparse
# import traceback
# from pathlib import Path
# from typing import Optional
# from datasets import Dataset, DatasetDict, load_dataset, Features, Value, List
# from dotenv import load_dotenv
# load_dotenv()

# # Configuration
# CWEVAL_REPO = "data/CWEval/CWEval-repo"
# OUTPUT_DIR = "data/CWEval/data"
# LANGUAGES = {
#     'c': '.c',
#     'cpp': '.cpp',
#     'go': '.go',
#     'js': '.js',
#     'py': '.py'
# }
# BENCHMARK_TYPES = ['core', 'lang']


# def extract_cwe_id_from_filename(filename):
#     """Extract CWE ID from filename like cwe_020_0_go_task.go"""
#     match = re.search(r'cwe_(\d+)_', filename)
#     if match:
#         return int(match.group(1))
#     return -1


# def read_file_content(filepath):
#     """Read file content, return empty string if file doesn't exist"""
#     try:
#         with open(filepath, 'r', encoding='utf-8') as f:
#             return f.read()
#     except Exception as e:
#         print(f"Warning: Could not read {filepath}: {e}")
#         return ""


# def extract_task_description(task_content, lang):
#     """
#     Extract the task description (docstring/comment) from the task file.
#     This is the part between BEGIN PROMPT and BEGIN SOLUTION.
#     """
#     # Different comment patterns for different languages
#     if lang in ['c', 'cpp', 'js', 'go']:
#         # For C, C++, JS, Go - look for /** ... */ or // comments
#         # Find BEGIN PROMPT marker
#         begin_prompt_match = re.search(r'//\s*BEGIN PROMPT', task_content)
#         begin_solution_match = re.search(r'//\s*BEGIN SOLUTION', task_content)
        
#         if begin_prompt_match and begin_solution_match:
#             start_idx = begin_prompt_match.end()
#             end_idx = begin_solution_match.start()
#             description = task_content[start_idx:end_idx].strip()
            
#             # Clean up comment markers
#             lines = description.split('\n')
#             cleaned_lines = []
#             for line in lines:
#                 # Remove leading comment markers
#                 line = re.sub(r'^\s*(/\*\*?|\*/?|//)\s?', '', line)
#                 cleaned_lines.append(line)
            
#             return '\n'.join(cleaned_lines).strip()
#         elif begin_solution_match:
#             print("Warning: BEGIN PROMPT marker not found. Extracting everything before BEGIN SOLUTION.")
#             before_solution = task_content[:begin_solution_match.start()]
#             return before_solution.strip()
#         elif begin_prompt_match:
#             print("Warning: BEGIN SOLUTION marker not found. Extracting everything after BEGIN PROMPT. This may include solution code.")
#             after_prompt = task_content[begin_prompt_match.end():]
#             return after_prompt.strip()
#         else:
#             print("Warning: Neither BEGIN PROMPT nor BEGIN SOLUTION markers found. Returning full content.")
#             return task_content.strip()
        
    
#     elif lang == 'py':
#         # For Python - look for docstring
#         # Find the docstring between BEGIN PROMPT and BEGIN SOLUTION markers
#         begin_solution_match = re.search(r'#\s*BEGIN SOLUTION', task_content)
        
#         if begin_solution_match:
#             # Look for docstring before BEGIN SOLUTION
#             before_solution = task_content[:begin_solution_match.start()]
            
#             # Find triple-quoted docstring
#             docstring_match = re.search(r"'''(.*?)'''", before_solution, re.DOTALL)
#             if not docstring_match:
#                 docstring_match = re.search(r'"""(.*?)"""', before_solution, re.DOTALL)
            
#             if docstring_match:
#                 return docstring_match.group(1).strip()
#         else :
#             print("Warning: BEGIN SOLUTION marker not found. Attempting to extract docstring from full content.")
#             # Look for triple-quoted docstring in full content
#             docstring_match = re.search(r"'''(.*?)'''", task_content, re.DOTALL)
#             if not docstring_match:
#                 docstring_match = re.search(r'"""(.*?)"""', task_content, re.DOTALL)
            
#             if docstring_match:
#                 print("Extracted docstring from full content.")
#                 return docstring_match.group(1).strip()
    
#     # Fallback: return everything before BEGIN SOLUTION
#     begin_solution_match = re.search(r'(//|#)\s*BEGIN SOLUTION', task_content)
#     if begin_solution_match:
#         return task_content[:begin_solution_match.start()].strip()
    
#     return task_content.strip()


# def extract_solution_code(task_content, lang):
#     """
#     Extract the solution code from the task file.
#     This is the part between BEGIN SOLUTION and BEGIN ENTRYPOINT (or end of file).
#     """
#     # Find BEGIN SOLUTION marker
#     if lang in ['c', 'cpp', 'js', 'go']:
#         begin_solution_match = re.search(r'//\s*BEGIN SOLUTION', task_content)
#         begin_entrypoint_match = re.search(r'//\s*BEGIN ENTRYPOINT', task_content)
#     else:  # python
#         begin_solution_match = re.search(r'#\s*BEGIN SOLUTION', task_content)
#         begin_entrypoint_match = re.search(r'#\s*BEGIN ENTRYPOINT', task_content)
    
#     if begin_solution_match:
#         start_idx = begin_solution_match.end()
#         if begin_entrypoint_match:
#             end_idx = begin_entrypoint_match.start()
#         else:
#             print(f"Warning: BEGIN ENTRYPOINT marker not found. Using end of file for solution code end.")
#             end_idx = len(task_content)
        
#         solution_code = task_content[start_idx:end_idx].strip()
#         return solution_code
    
#     print(f"Warning: BEGIN SOLUTION marker not found.")
#     return task_content.strip()


# def create_prompt_from_task(task_content, lang):
#     """Create a prompt from the task file - just the description, no tests or solution"""
#     task_description = extract_task_description(task_content, lang)
#     task_description = task_description.strip()
    
#     # Return as JSON format expected by the system
#     return json.dumps([{"role": "user", "content": task_description}])


# def create_completion_from_code(code):
#     """Wrap code in <code> tags and create completion format"""
#     code = code.strip()
#     code_with_tags = f"<code>\n{code}\n</code>"
#     return json.dumps([{"role": "assistant", "content": code_with_tags}])


# def process_benchmark_directory(benchmark_type, lang):
#     """Process a benchmark directory (core or lang) for a specific language"""
#     print(f"\nProcessing {benchmark_type}/{lang}...")
    
#     benchmark_dir = os.path.join(CWEVAL_REPO, "benchmark", benchmark_type, lang)
    
#     if not os.path.exists(benchmark_dir):
#         print(f"  Directory not found: {benchmark_dir}")
#         return None
    
#     # Find all task files
#     task_files = []
#     for file in os.listdir(benchmark_dir):
#         if file.endswith(f"_task{LANGUAGES[lang]}"):
#             task_files.append(file)
    
#     if not task_files:
#         print(f"  No task files found in {benchmark_dir}")
#         return None
    
#     print(f"  Found {len(task_files)} task files")
    
#     # Process each task file
#     output_data = []
#     for task_file in sorted(task_files):
#         # Extract base name and CWE ID
#         base_name = task_file.replace(f"_task{LANGUAGES[lang]}", "")
#         cwe_id = extract_cwe_id_from_filename(task_file)
        
#         # Construct paths
#         task_path = os.path.join(benchmark_dir, task_file)
        
#         # Read task file
#         task_content = read_file_content(task_path)
        
#         if not task_content:
#             print(f"  Warning: Empty task file {task_file}")
#             continue
        
#         # Extract task description (for prompt)
#         prompt = create_prompt_from_task(task_content, lang)
        
#         # Extract solution code (for completion and y_negative)
#         solution_code = extract_solution_code(task_content, lang)
#         completion = create_completion_from_code(solution_code)
#         y_negative = f"<code>\n{solution_code}\n</code>"
        
#         # Create unique ID
#         task_id = f"CWEval_{benchmark_type}_{lang}_{base_name}"
        
#         # Create record
#         record = {
#             'task_id': task_id,
#             'id': base_name,
#             'CWE_ID': cwe_id,
#             'prompt': prompt,
#             'completion': completion,
#             'y_negative': y_negative,  # The vulnerable code in <code> tags
#             'cot_steps': ''  # Empty initially, to be filled by generate_cot script
#         }
        
#         output_data.append(record)
    
#     if not output_data:
#         return None
    
#     # Create DataFrame
#     df = pd.DataFrame(output_data)
    
#     # Save to CSV
#     output_filename = f"CWEval_{benchmark_type}_{lang}.csv"
#     output_path = os.path.join(OUTPUT_DIR, output_filename)
#     df.to_csv(output_path, index=False)
    
#     print(f"  ✓ Saved {len(df)} examples to: {output_path}")
    
#     return df


# def validate_and_push_to_hf(split_name: str, csv_path: str, hf_dataset_name: str, hf_token: str):
#     """
#     Validate schema and push a single split to HuggingFace Hub.
    
#     Args:
#         split_name: Split name (e.g., "core_c", "lang_py")
#         csv_path: Path to CSV file
#         hf_dataset_name: HuggingFace dataset name
#         hf_token: HuggingFace token
    
#     Returns:
#         bool: True if successful, False otherwise
#     """
#     try:
#         print(f"\n{'='*60}")
#         print(f"Validating and Pushing {split_name} to HuggingFace Hub")
#         print(f"{'='*60}")
        
#         # Load CSV
#         print(f"Loading {csv_path}...")
#         df = pd.read_csv(csv_path)
        
#         # Parse JSON strings to proper format
#         print("Converting JSON strings to proper format...")
        
#         def parse_messages(json_str):
#             """Parse JSON string to list of message dicts"""
#             if pd.isna(json_str) or json_str == '':
#                 return []
#             try:
#                 messages = json.loads(json_str)
#                 return messages
#             except Exception as e:
#                 print(f"Warning: Could not parse JSON: {json_str[:100]}... Error: {e}")
#                 return []
        
#         df['prompt'] = df['prompt'].apply(parse_messages)
#         df['completion'] = df['completion'].apply(parse_messages)
        
#         # Convert id to string
#         df['id'] = df['id'].astype(str)
        
#         # Define the reference schema
#         ref_features = Features({
#             'task_id': Value('string'),
#             'id': Value('string'),
#             'CWE_ID': Value('int64'),
#             'y_negative': Value('string'),
#             'prompt': List({'content': Value('string'), 'role': Value('string')}),
#             'cot_steps': Value('string'),
#             'completion': List({'content': Value('string'), 'role': Value('string')})
#         })
        
#         print("\nCreating dataset with schema validation...")
#         # Convert to HuggingFace Dataset with proper schema
#         dataset = Dataset.from_pandas(df, features=ref_features)
        
#         # Ensure required columns are present
#         required_columns = ['task_id', 'id', 'CWE_ID', 'prompt', 'completion', 'y_negative', 'cot_steps']
#         available_columns = [col for col in required_columns if col in dataset.column_names]
#         dataset = dataset.select_columns(available_columns)
        
#         # Validate features match reference
#         print("\n" + "="*60)
#         print("Feature Validation")
#         print("="*60)
        
#         print("\nExpected Features:")
#         for key, value in ref_features.items():
#             print(f"  {key}: {value}")
        
#         print("\nActual Features:")
#         for key, value in dataset.features.items():
#             print(f"  {key}: {value}")
        
#         # Assert features match using direct comparison
#         print("\nValidating schema...")
#         assert dataset.features == ref_features, \
#             f"Features mismatch!\n  Expected: {ref_features}\n  Actual: {dataset.features}"
        
#         print("✓ All features validated successfully!")
        
#         # Print sample data
#         print(f"\nDataset size: {len(dataset)} examples")
#         print("\nFirst example (truncated):")
#         example = dataset[0]
#         for key, value in example.items():
#             if isinstance(value, str):
#                 display_value = value[:100] + "..." if len(value) > 100 else value
#             elif isinstance(value, list):
#                 display_value = f"[{len(value)} items]"
#             else:
#                 display_value = value
#             print(f"  {key}: {display_value}")
        
#         # Try to load existing dataset to preserve other splits
#         try:
#             print(f"\nLoading existing dataset {hf_dataset_name}...")
#             existing_dataset = load_dataset(hf_dataset_name, token=hf_token)
#             print(f"Found existing dataset with splits: {list(existing_dataset.keys())}")
            
#             # Add or update the current split
#             dataset_dict = DatasetDict(existing_dataset)
#             dataset_dict[split_name] = dataset
            
#             print(f"\nPushing updated dataset with {split_name} split...")
#             dataset_dict.push_to_hub(
#                 hf_dataset_name,
#                 token=hf_token,
#                 private=False,
#                 commit_message=f"Add/update {split_name} split - CWEval security dataset"
#             )
#         except Exception as e:
#             print(f"Could not load existing dataset (might be first push): {e}")
#             print(f"Creating new DatasetDict with {split_name} split...")
            
#             dataset_dict = DatasetDict({split_name: dataset})
            
#             dataset_dict.push_to_hub(
#                 hf_dataset_name,
#                 token=hf_token,
#                 private=False,
#                 commit_message=f"Initial upload with {split_name} split - CWEval security dataset"
#             )
        
#         print(f"\n✓ Successfully pushed {split_name} to https://huggingface.co/datasets/{hf_dataset_name}")
#         return True
        
#     except AssertionError as e:
#         print(f"\n✗ Schema validation failed for {split_name}!")
#         print(f"Error: {e}")
#         print(f"\n⚠ CSV is saved at {csv_path} but NOT pushed to HuggingFace")
#         return False
#     except Exception as e:
#         print(f"\n✗ Error processing {split_name}: {e}")
#         traceback.print_exc()
#         return False


# def main():
#     """Main processing function"""
#     parser = argparse.ArgumentParser(description='Create CWEval dataset from repository')
#     parser.add_argument('--push-to-hf', action='store_true',
#                        help='Push datasets to HuggingFace Hub after creation')
#     parser.add_argument('--hf-dataset', type=str, default='ShethArihant/CWEval-v1',
#                        help='HuggingFace dataset name')
    
#     args = parser.parse_args()
    
#     print("="*60)
#     print("CWEval Dataset Preprocessing")
#     print("="*60)
#     print(f"Repository: {CWEVAL_REPO}")
#     print(f"Output directory: {OUTPUT_DIR}")
#     print(f"Languages: {', '.join(LANGUAGES.keys())}")
#     print(f"Benchmark types: {', '.join(BENCHMARK_TYPES)}")
#     print(f"Push to HuggingFace: {args.push_to_hf}")
#     if args.push_to_hf:
#         print(f"HuggingFace dataset: {args.hf_dataset}")
    
#     # Check if repository exists
#     if not os.path.exists(CWEVAL_REPO):
#         print(f"\nError: Repository not found at {CWEVAL_REPO}")
#         return
    
#     # Create output directory
#     os.makedirs(OUTPUT_DIR, exist_ok=True)
    
#     # Get HF token if pushing
#     hf_token = None
#     if args.push_to_hf:
#         hf_token = os.environ.get('HF_TOKEN', None)
#         if not hf_token:
#             print("\n⚠ Warning: HF_TOKEN not found in environment. Cannot push to HuggingFace.")
#             args.push_to_hf = False
    
#     # Process each benchmark type and language
#     results = {}
#     push_results = {}
    
#     for benchmark_type in BENCHMARK_TYPES:
#         for lang in LANGUAGES.keys():
#             key = f"{benchmark_type}_{lang}"
#             result = process_benchmark_directory(benchmark_type, lang)
#             if result is not None:
#                 results[key] = result
                
#                 # Push to HuggingFace if enabled
#                 if args.push_to_hf and hf_token:
#                     csv_path = os.path.join(OUTPUT_DIR, f"CWEval_{benchmark_type}_{lang}.csv")
#                     push_success = validate_and_push_to_hf(key, csv_path, args.hf_dataset, hf_token)
#                     push_results[key] = push_success
    
#     # Summary
#     print("\n" + "="*60)
#     print("SUMMARY")
#     print("="*60)
    
#     for benchmark_type in BENCHMARK_TYPES:
#         print(f"\n{benchmark_type.upper()}:")
#         for lang in LANGUAGES.keys():
#             key = f"{benchmark_type}_{lang}"
#             if key in results:
#                 status = ""
#                 if args.push_to_hf:
#                     if key in push_results and push_results[key]:
#                         status = " ✓ pushed to HF"
#                     else:
#                         status = " ✗ push failed"
#                 print(f"  {lang:10} {len(results[key]):5} examples{status}")
#             else:
#                 print(f"  {lang:10}     0 examples")
    
#     total_examples = sum(len(df) for df in results.values())
#     print(f"\nTotal examples across all datasets: {total_examples}")
    
#     if args.push_to_hf:
#         successful_pushes = sum(1 for v in push_results.values() if v)
#         print(f"Successfully pushed: {successful_pushes}/{len(push_results)} datasets")
    
#     print("\n✓ CWEval dataset preprocessing complete!")


# if __name__ == "__main__":
#     main()