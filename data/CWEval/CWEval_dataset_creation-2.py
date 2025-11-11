import os
import json
import pandas as pd
import re
from pathlib import Path

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
BENCHMARK_TYPES = ['core', 'lang']

os.makedirs(OUTPUT_DIR, exist_ok=True)

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

def create_prompt_from_task(task_content, test_content, lang, cwe_id):
    """Create a prompt from the task file"""
    # prompt_text = f"Write secure {lang} code that passes the following tests and avoids CWE-{cwe_id} vulnerabilities.\n\n"
    # prompt_text += f"Task:\n{task_content}\n\n"
    # prompt_text += f"Tests:\n{test_content}"
    prompt_text = task_content + "\n\n" + test_content
    
    return json.dumps([{"role": "user", "content": prompt_text}])

def create_completion_from_task(task_content):
    """Create completion from task file (the actual code)"""
    return json.dumps([{"role": "assistant", "content": task_content}])

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
        test_path = os.path.join(benchmark_dir, f"{base_name}_test.py")
        
        # Read files
        task_content = read_file_content(task_path)
        test_content = read_file_content(test_path)
        
        if not task_content:
            print(f"  Warning: Empty task file {task_file}")
            continue
        
        # Create unique ID
        task_id = f"CWEval_{benchmark_type}_{lang}_{base_name}"
        
        # Create prompt and completion
        prompt = create_prompt_from_task(task_content, test_content, lang, cwe_id)
        completion = create_completion_from_task(task_content)
        
        # Create record
        record = {
            'task_id': task_id,
            'id': base_name,
            'CWE_ID': cwe_id,
            'prompt': prompt,
            'completion': completion,
            'y_negative': task_content,  # The task file contains the vulnerable code
            'cot_steps': ''
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

def main():
    """Main processing function"""
    print("="*60)
    print("CWEval Dataset Preprocessing")
    print("="*60)
    print(f"Repository: {CWEVAL_REPO}")
    print(f"Output directory: {OUTPUT_DIR}")
    print(f"Languages: {', '.join(LANGUAGES.keys())}")
    print(f"Benchmark types: {', '.join(BENCHMARK_TYPES)}")
    
    # Check if repository exists
    if not os.path.exists(CWEVAL_REPO):
        print(f"\nError: Repository not found at {CWEVAL_REPO}")
        return
    
    # Process each benchmark type and language
    results = {}
    for benchmark_type in BENCHMARK_TYPES:
        for lang in LANGUAGES.keys():
            key = f"{benchmark_type}_{lang}"
            result = process_benchmark_directory(benchmark_type, lang)
            if result is not None:
                results[key] = result
    
    # Summary
    print("\n" + "="*60)
    print("SUMMARY")
    print("="*60)
    
    for benchmark_type in BENCHMARK_TYPES:
        print(f"\n{benchmark_type.upper()}:")
        for lang in LANGUAGES.keys():
            key = f"{benchmark_type}_{lang}"
            if key in results:
                print(f"  {lang:10} {len(results[key]):5} examples")
            else:
                print(f"  {lang:10}     0 examples")
    
    total_examples = sum(len(df) for df in results.values())
    print(f"\nTotal examples across all datasets: {total_examples}")
    print("\n✓ CWEval dataset preprocessing complete!")

if __name__ == "__main__":
    main()