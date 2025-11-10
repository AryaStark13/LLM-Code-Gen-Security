import pandas as pd
import os
import sys
import json
from typing import Optional
from tqdm.auto import tqdm
import random
from concurrent.futures import ThreadPoolExecutor, as_completed
import argparse
from functools import partial
from datasets import Dataset, DatasetDict, load_dataset, Features, Value, List
import traceback

# Setup
random.seed(42)
tqdm.pandas()

from dotenv import load_dotenv
load_dotenv()

# Import your LLM client
from utils.llms import OpenAI


# Language to code block mapping
LANG_TO_CODE_BLOCK = {
    'python': 'python',
    'javascript': 'javascript',
    'java': 'java',
    'php': 'php',
    'ruby': 'ruby',
    'go': 'go'
}


def get_structured_instruction(lang: str) -> str:
    """Get the standard instruction format for structured prompts."""
    code_block_lang = LANG_TO_CODE_BLOCK.get(lang, lang)
    return (
        "Important: Write your reasoning steps within <think> and </think> tags. "
        f"And wrap your final code implementation within ```{code_block_lang} and ``` tags.\n"
        "Example format:\n"
        "<think>Your security reasoning steps here...</think>\n"
        f"```{code_block_lang}\n"
        "Your final secure code implementation here...\n"
        "```"
    )


def generate_structured_prompt(row: pd.Series, lang: str) -> str:
    """Generate structured markdown format prompt from CSV row."""
    
    security_instructions = (
        "Think about the security implications of this problem carefully and step-by-step. "
        "Then implement secure code that addresses the vulnerability and meets the requirements described. "
        "Add imports if necessary and follow security best practices."
    )
    
    # Parse the prompt from JSON
    prompt_data = json.loads(row['prompt'])
    actual_prompt = prompt_data[0]['content']
    
    prompt_parts = []
    
    prompt_parts.append(f"### Security Instructions:\n{security_instructions}\n")
    prompt_parts.append(f"**Problem:**\n{actual_prompt}\n")
    
    # Add standard instruction
    prompt_parts.append(get_structured_instruction(lang) + "\n")
    
    return "\n".join(prompt_parts)


def generate_cot_prompt(X: str, y_positive: str, y_negative: str, vulnerability: str = None) -> str:
    """
    Generate reasoning prompt for the larger model with security focus.
    
    Args:
        X: Input prompt
        y_positive: Safe/secure code implementation
        y_negative: Unsafe/vulnerable code implementation
        vulnerability: Description of the vulnerability
    
    Returns:
        CoT generation prompt
    """
    vulnerability_context = f"\n**Vulnerability Context:** {vulnerability}\n" if vulnerability else ""
    
    cot_prompt = f"""{X}

Here is the secure code implementation:
{y_positive}

Here is the vulnerable code implementation (DO NOT use this):
{y_negative}
{vulnerability_context}
Let's reason through this security problem step by step. Explain your thought process to solve the above problem securely.

Your reasoning should include:
1. What security vulnerability or risk exists in the problem
2. How the secure implementation addresses this vulnerability
3. Why the vulnerable code is unsafe (what attack vectors it enables)
4. What security best practices are being followed

Do NOT provide any details of the actual code implementation in your reasoning.
Only include the reasoning, no other text.
Important: Be concise and to the point in your reasoning. Think step by step.
"""
    return cot_prompt


def generate_cot(X: str, y_positive: str, y_negative: str, vulnerability: str, llm_client) -> str:
    """Generate CoT response from larger model."""
    cot_prompt = generate_cot_prompt(X, y_positive, y_negative, vulnerability)
    try:
        llm_response, llm_response_text = llm_client.send_message(cot_prompt)
        return llm_response_text
    except Exception as e:
        print(f"Error generating CoT: {e}")
        return ""


def process_row(row: pd.Series, lang: str, llm_client) -> pd.Series:
    """Process a single row to generate X and cot_steps."""
    
    # Generate X (the prompt)
    X = generate_structured_prompt(row, lang)
    
    # Parse completion to get safe code
    completion_data = json.loads(row['completion'])
    safe_code = completion_data[0]['content']
    
    # Get vulnerable code from y_negative
    vulnerable_code = row['y_negative']
    
    # Get vulnerability description (may be NaN)
    vulnerability = row.get('vulnerability', None)
    if pd.isna(vulnerability):
        vulnerability = None
    
    # Generate CoT using larger model with security context
    cot_text = generate_cot(X, safe_code, vulnerable_code, vulnerability, llm_client)
    cot_steps = f"<think>{cot_text}</think>\n" if cot_text else ""
    
    # Update the row
    row['prompt'] = json.dumps([{"role": "user", "content": X}])
    # completion stays as is (safe code) - handled in train script
    row['cot_steps'] = cot_steps
    
    return row


def process_single_row_wrapper(args):
    """Wrapper for parallel processing of single rows."""
    idx, row, lang, model_name = args
    # Create a new LLM client for this thread
    llm_client = OpenAI(model_name=model_name)
    try:
        processed_row = process_row(row, lang, llm_client)
        return idx, processed_row, None
    except Exception as e:
        return idx, row, str(e)


def generate_dataset_with_cot(input_csv: str, output_csv: str, lang: str, 
                               model_name: str = "gpt-4o-2024-08-06",
                               parallel: bool = False, max_workers: int = 4):
    """
    Read CSV, generate CoT for each row, and save to new CSV.
    
    Args:
        input_csv: Path to input CSV file
        output_csv: Path to output CSV file
        lang: Programming language
        model_name: LLM model name
        parallel: Whether to use parallel processing
        max_workers: Number of parallel workers
    """
    print(f"\n{'='*60}")
    print(f"Processing {lang.upper()} dataset")
    print(f"{'='*60}")
    print(f"Reading dataset from {input_csv}...")
    df = pd.read_csv(input_csv)
    
    print(f"Total rows: {len(df)}")
    print(f"Columns: {df.columns.tolist()}")
    
    if parallel:
        print(f"\nGenerating CoT using {max_workers} parallel workers...")
        
        # Prepare arguments for parallel processing
        args_list = [(idx, row, lang, model_name) for idx, row in df.iterrows()]
        
        processed_rows = [None] * len(df)
        errors = []
        
        with ThreadPoolExecutor(max_workers=max_workers) as executor:
            # Submit all tasks
            futures = {executor.submit(process_single_row_wrapper, args): args[0] 
                      for args in args_list}
            
            # Process completed tasks with progress bar
            for future in tqdm(as_completed(futures), total=len(futures), 
                             desc=f"Generating CoT for {lang}"):
                idx, processed_row, error = future.result()
                
                if error:
                    print(f"\nError processing row {idx}: {error}")
                    processed_row['cot_steps'] = ""
                    errors.append((idx, error))
                
                processed_rows[idx] = processed_row
        
        if errors:
            print(f"\n⚠ {len(errors)} rows had errors during processing")
    else:
        print(f"\nGenerating CoT sequentially...")
        
        # Create LLM client once for sequential processing
        llm_client = OpenAI(model_name=model_name)
        
        # Process each row with progress bar
        processed_rows = []
        for idx, row in tqdm(df.iterrows(), total=len(df), desc=f"Generating CoT for {lang}"):
            try:
                processed_row = process_row(row, lang, llm_client)
                processed_rows.append(processed_row)
            except Exception as e:
                print(f"\nError processing row {idx}: {e}")
                # Add the row with empty CoT values
                row['cot_steps'] = ""
                processed_rows.append(row)
    
    # Create new dataframe
    result_df = pd.DataFrame(processed_rows)
    
    print(f"\nSaving to {output_csv}...")
    result_df.to_csv(output_csv, index=False)
    
    print(f"✓ Saved {len(result_df)} entries to {output_csv}")
    
    # Print statistics
    non_empty_cot = (result_df['cot_steps'] != '').sum()
    print(f"  - {non_empty_cot}/{len(result_df)} examples have CoT ({non_empty_cot/len(result_df)*100:.1f}%)")
    
    return result_df


def validate_and_push_to_hf(lang: str, csv_path: str, hf_dataset_name: str, hf_token: str):
    """
    Validate schema and push a single language dataset to HuggingFace Hub.
    
    Args:
        lang: Programming language
        csv_path: Path to CSV file
        hf_dataset_name: Base HuggingFace dataset name
        hf_token: HuggingFace token
    """    
    try:
        print(f"\n{'='*60}")
        print(f"Validating and Pushing {lang.upper()} to HuggingFace Hub")
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
        
        # Try to load existing dataset to preserve other languages
        try:
            print(f"\nLoading existing dataset {hf_dataset_name}...")
            existing_dataset = load_dataset(hf_dataset_name, token=hf_token)
            print(f"Found existing dataset with splits: {list(existing_dataset.keys())}")
            
            # Add or update the current language split
            dataset_dict = DatasetDict(existing_dataset)
            dataset_dict[lang] = dataset
            
            print(f"\nPushing updated dataset with {lang} split...")
            dataset_dict.push_to_hub(
                hf_dataset_name,
                token=hf_token,
                private=False,
                commit_message=f"Add/update {lang} split with security CoT reasoning"
            )
        except Exception as e:
            print(f"Could not load existing dataset (might be first push): {e}")
            print(f"Creating new DatasetDict with {lang} split...")
            
            dataset_dict = DatasetDict({lang: dataset})
            
            dataset_dict.push_to_hub(
                hf_dataset_name,
                token=hf_token,
                private=False,
                commit_message=f"Initial upload with {lang} split - security focused DPO dataset"
            )
        
        print(f"\n✓ Successfully pushed {lang} to https://huggingface.co/datasets/{hf_dataset_name}")
        return True
        
    except AssertionError as e:
        print(f"\n✗ Schema validation failed for {lang}!")
        print(f"Error: {e}")
        print(f"\n⚠ CSV is saved at {csv_path} but NOT pushed to HuggingFace")
        return False
    except Exception as e:
        print(f"\n✗ Error processing {lang}: {e}")
        traceback.print_exc()
        return False


def process_single_language(lang: str, input_data_dir: str, output_data_dir: str,
                            hf_dataset_name: str, hf_token: Optional[str],
                            model_name: str, parallel: bool, max_workers: int,
                            push_eager: bool = True):
    """
    Process a single language: generate CoT, validate schema, and optionally push to HF.
    
    Args:
        lang: Programming language
        input_data_dir: Directory with input CSV files
        output_data_dir: Directory for output CSV files
        hf_dataset_name: HuggingFace dataset name
        hf_token: HuggingFace token
        model_name: LLM model name
        parallel: Whether to use parallel processing for rows
        max_workers: Number of parallel workers
        push_eager: Whether to push immediately after processing
    
    Returns:
        Tuple of (success: bool, lang: str, output_path: str)
    """
    input_csv = os.path.join(input_data_dir, f"CyberNative-DPO_dataset_{lang}.csv")
    output_csv = os.path.join(output_data_dir, f"CyberNative-DPO_dataset_{lang}_with_cot.csv")
    
    if not os.path.exists(input_csv):
        print(f"\n⚠ Warning: {input_csv} not found, skipping {lang}")
        return False, lang, None
    
    try:
        # Step 1: Generate CoT and save CSV
        df_with_cot = generate_dataset_with_cot(
            input_csv, output_csv, lang, 
            model_name=model_name,
            parallel=parallel, 
            max_workers=max_workers
        )
        
        print(f"\n✓ CSV saved successfully: {output_csv}")
        
        # Step 2: Validate schema and push to HuggingFace (if enabled and token available)
        if push_eager and hf_token:
            push_success = validate_and_push_to_hf(lang, output_csv, hf_dataset_name, hf_token)
            if not push_success:
                print(f"⚠ Warning: Validation or push failed for {lang}, but CSV is saved locally")
        elif not hf_token:
            print(f"⚠ No HF token provided, skipping push for {lang}")
        else:
            print(f"⚠ Eager push disabled, skipping push for {lang}")
        
        return True, lang, output_csv
        
    except Exception as e:
        print(f"\n✗ Error processing {lang}: {e}")
        traceback.print_exc()
        return False, lang, None


def main():
    parser = argparse.ArgumentParser(description='Generate security-focused CoT for CyberNative-DPO dataset')
    parser.add_argument('--languages', nargs='+', 
                       default=['python', 'java', 'javascript', 'php', 'ruby', 'go'],
                       help='List of languages to process')
    parser.add_argument('--parallel-rows', action='store_true',
                       help='Use parallel processing for rows within each language')
    parser.add_argument('--max-row-workers', type=int, default=4,
                       help='Number of parallel workers for row processing')
    parser.add_argument('--no-eager-push', action='store_true',
                       help='Disable eager pushing to HuggingFace after each language')
    parser.add_argument('--model', type=str, default='gpt-4o-2024-08-06',
                       help='LLM model name')
    parser.add_argument('--input-dir', type=str, default='data/CyberNative-DPO/data',
                       help='Input directory with base datasets')
    parser.add_argument('--output-dir', type=str, default='data/CyberNative-DPO/data/cot_data',
                       help='Output directory for datasets with CoT')
    parser.add_argument('--hf-dataset', type=str, default='ShethArihant/CyberNative-DPO-v1',
                       help='HuggingFace dataset name')
    
    args = parser.parse_args()
    
    # Configuration
    languages = args.languages
    input_data_dir = args.input_dir
    output_data_dir = args.output_dir
    hf_dataset_name = args.hf_dataset
    model_name = args.model
    parallel_rows = args.parallel_rows
    max_row_workers = args.max_row_workers
    push_eager = not args.no_eager_push
    
    # Create output directory
    os.makedirs(output_data_dir, exist_ok=True)
    
    # Get HF token
    hf_token = os.environ.get('HF_TOKEN', None)
    
    if push_eager and not hf_token:
        print("⚠ Warning: HF_TOKEN not found. Eager pushing disabled.")
        push_eager = False
    
    print("="*60)
    print("CyberNative-DPO CoT Generation Configuration")
    print("="*60)
    print(f"Languages: {', '.join(languages)}")
    print(f"Model: {model_name}")
    print(f"Parallel row processing: {parallel_rows}")
    if parallel_rows:
        print(f"  Max row workers: {max_row_workers}")
    print(f"Eager push to HuggingFace: {push_eager}")
    print(f"Input directory: {input_data_dir}")
    print(f"Output directory: {output_data_dir}")
    print(f"HuggingFace dataset: {hf_dataset_name}")
    print("\nSecurity Focus: CoT will include vulnerability analysis")
    
    # Step 1: Generate CoT for each language dataset
    print("\n" + "="*60)
    print("Generating Security-Focused CoT for all languages")
    print("="*60)
    
    results = []
    
    print(f"\nProcessing {len(languages)} languages sequentially...")
    
    for lang in languages:
        success, lang, output_path = process_single_language(
            lang=lang,
            input_data_dir=input_data_dir,
            output_data_dir=output_data_dir,
            hf_dataset_name=hf_dataset_name,
            hf_token=hf_token,
            model_name=model_name,
            parallel=parallel_rows,
            max_workers=max_row_workers,
            push_eager=push_eager
        )
        results.append((success, lang, output_path))
    
    # Final summary
    print("\n" + "="*60)
    print("Summary")
    print("="*60)
    
    successful = [r for r in results if r[0]]
    failed = [r for r in results if not r[0]]
    
    print(f"\n✓ Successfully processed: {len(successful)}/{len(languages)} languages")
    for success, lang, output_path in successful:
        if output_path:
            df = pd.read_csv(output_path)
            non_empty = (df['cot_steps'] != '').sum()
            print(f"  - {lang:12} {len(df):5} examples ({non_empty} with CoT)")
    
    if failed:
        print(f"\n✗ Failed: {len(failed)} languages")
        for success, lang, output_path in failed:
            print(f"  - {lang}")
    
    print(f"\n{'='*60}")
    print("✓ Complete!")
    print(f"{'='*60}")
    print(f"CSV files with CoT: {output_data_dir}")
    if push_eager and hf_token:
        print(f"HuggingFace Dataset: https://huggingface.co/datasets/{hf_dataset_name}")
        print("\n⚠ Note: Only languages that passed schema validation were pushed")
    else:
        print(f"HuggingFace Dataset: Not pushed (use HF_TOKEN or remove --no-eager-push)")
    
    return results


if __name__ == "__main__":
    results = main()