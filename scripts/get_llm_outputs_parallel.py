from datasets import load_dataset
import pandas as pd
import sys
import os
import argparse
import json
from datetime import datetime
from tqdm import tqdm
from concurrent.futures import ThreadPoolExecutor, as_completed
import time

sys.path.append(os.path.abspath(".."))
from utils.llms import OpenAI


def extract_code_from_response(response):
    """Extract code from the response if it's wrapped in tags."""
    if isinstance(response, str):
        # Try to extract code between <code> tags
        import re
        code_match = re.search(r'<code>(.*?)</code>', response, re.DOTALL)
        if code_match:
            return code_match.group(1).strip()
    return response


def process_single_example(idx, example, llm_client, split_name, max_retries=3, retry_delay=2):
    """Process a single example with retry logic."""
    for attempt in range(max_retries):
        try:
            # Extract the prompt
            prompt = example["prompt"][0]["content"] if isinstance(example["prompt"], list) else example["prompt"]
            
            # Get response from the model
            response, message = llm_client.send_message(prompt)
            ground_truth_cot = example.get("cot_steps", "")
            if ground_truth_cot is None:
                ground_truth_cot = ""
            
            # Build the result object
            result = {
                "split": split_name,
                "index": idx,
                "task_id": example.get("task_id", ""),
                "id": example.get("id", idx),
                "CWE_ID": example.get("CWE_ID", None),
                "prompt": prompt,
                "ground_truth_cot": ground_truth_cot,
                "ground_truth_code": example.get("completion", "")[0]["content"],
                "ground_truth_full": ground_truth_cot + example.get("completion", "")[0]["content"],
                "y_negative": example.get("y_negative", ""),
                "output_without_tuning": "",
                "output_with_tuning": message,
                "processed_at": idx + 1,
                "has_non_tuned_output": False
            }
            
            return result, None
            
        except Exception as e:
            if attempt < max_retries - 1:
                print(f"\nError processing example {idx} (attempt {attempt + 1}/{max_retries}): {str(e)}. Retrying...")
                time.sleep(retry_delay)
            else:
                print(f"\nError processing example {idx} after {max_retries} attempts: {str(e)}")
                # Return error result after all retries exhausted
                ground_truth_cot = ""
                result = {
                    "split": split_name,
                    "index": idx,
                    "id": example.get("id", idx),
                    "task_id": example.get("task_id", ""),
                    "CWE_ID": example.get("CWE_ID", None),
                    "prompt": example.get("prompt", ""),
                    "ground_truth_cot": ground_truth_cot,
                    "ground_truth_code": example.get("completion", "")[0]["content"] if example.get("completion") else "",
                    "ground_truth_full": ground_truth_cot + (example.get("completion", "")[0]["content"] if example.get("completion") else ""),
                    "y_negative": example.get("y_negative", ""),
                    "output_without_tuning": "",
                    "output_with_tuning": f"ERROR: {str(e)}",
                    "processed_at": idx + 1,
                    "has_non_tuned_output": False
                }
                return result, str(e)


def process_split(dataset, split, model_name, llm_client, max_workers=4, max_retries=3):
    """Process a single split with parallel processing."""
    
    # Load the dataset split
    print(f"\nProcessing split: {split}")
    eval_dataset = load_dataset(dataset, split=split)
    
    total_examples = len(eval_dataset)
    print(f"  Total examples: {total_examples}")
    
    results = [None] * total_examples
    errors = []
    
    # Process examples in parallel
    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        # Submit all tasks
        future_to_idx = {
            executor.submit(process_single_example, idx, example, llm_client, split, max_retries): idx
            for idx, example in enumerate(eval_dataset)
        }
        
        # Process completed tasks
        with tqdm(total=total_examples, desc=f"  {split}") as pbar:
            for future in as_completed(future_to_idx):
                idx = future_to_idx[future]
                try:
                    result, error = future.result()
                    results[idx] = result
                    if error:
                        errors.append((idx, error))
                except Exception as e:
                    print(f"\nUnexpected error processing example {idx}: {str(e)}")
                    errors.append((idx, str(e)))
                pbar.update(1)
    
    # Ensure all results are present
    processed_count = sum(1 for r in results if r is not None)
    
    if processed_count < total_examples:
        print(f"  Warning: Only {processed_count}/{total_examples} examples were processed successfully")
    
    return [r for r in results if r is not None], processed_count, errors


def process_dataset(dataset, splits, model_name, output_file=None, max_workers=4, max_retries=3, max_new_tokens=2048):
    """Process the entire evaluation dataset across multiple splits with the specified model using parallel processing."""
    
    # Initialize the LLM client
    print(f"Initializing {model_name}...")
    llm_client = OpenAI(model_name=model_name)
    
    print(f"Loading dataset: {dataset}")
    print(f"Splits to process: {', '.join(splits)}")
    print(f"Using {max_workers} parallel workers")
    print(f"Max retries per example: {max_retries}")
    
    all_results = []
    splits_processed = []
    total_examples_count = 0
    
    # Process each split
    for split in splits:
        try:
            split_results, processed_count, errors = process_split(
                dataset, split, model_name, llm_client, max_workers, max_retries
            )
            all_results.extend(split_results)
            splits_processed.append(split)
            total_examples_count += len(split_results)
            
            if errors:
                print(f"  ✗ {len(errors)} examples had errors in split {split}")
        except Exception as e:
            print(f"  ✗ Failed to process split {split}: {str(e)}")
    
    # Create the final output object
    output_data = {
        "total_splits": len(splits),
        "splits_processed": splits_processed,
        "total_examples": total_examples_count,
        "batch_size": max_workers,
        "max_new_tokens": max_new_tokens,
        "model_name": model_name,
        "dataset": dataset,
        "max_retries": max_retries,
        "timestamp": datetime.now().isoformat(),
        "results": all_results
    }
    
    # Determine output filename
    if output_file is None:
        raise ValueError("Output file path must be provided.")
    
    # check if output directory exists, if not create it
    output_dir = os.path.dirname(output_file)
    if output_dir and not os.path.exists(output_dir):
        # create parent directories as needed (same as mkdir -p)
        os.makedirs(output_dir, exist_ok=True)

    # Save to JSON file
    print(f"\nSaving results to {output_file}...")
    with open(output_file, 'w', encoding='utf-8') as f:
        json.dump(output_data, f, indent=2, ensure_ascii=False)
    
    print(f"\n✓ Successfully processed {len(splits_processed)}/{len(splits)} splits")
    print(f"✓ Total examples processed: {total_examples_count}")
    print(f"✓ Results saved to: {output_file}")
    
    return output_data


def main():
    parser = argparse.ArgumentParser(
        description="Evaluate a language model on a Dataset of our format with parallel processing."
    )
    parser.add_argument(
        "--dataset",
        type=str,
        default="ShethArihant/CWEval-v1",
        help="Name of the dataset to use (default: ShethArihant/CWEval-v1)"
    )
    parser.add_argument(
        "--splits",
        type=str,
        nargs='+',
        # set default as core_c, core_cpp, core_java, core_py, core_js, core_go, lang_c
        default=["core_c", "core_cpp", "core_java", "core_py", "core_js", "core_go", "lang_c"],
        help="Dataset splits to use (space-separated, default: Python). Example: --splits Python Java C++"
    )
    parser.add_argument(
        "--model_name",
        type=str,
        default="gpt-4o-2024-08-06",
        help="Name of the model to use (e.g., 'gpt-4o-2024-08-06')"
    )
    parser.add_argument(
        "-o", "--output_file",
        type=str,
        help="Output JSON file (with path) to save results. Should point to a file in the results/ directory."
    )
    parser.add_argument(
        "--max_workers",
        type=int,
        default=4,
        help="Maximum number of parallel workers (default: 4)"
    )
    parser.add_argument(
        "--max_retries",
        type=int,
        default=3,
        help="Maximum number of retries for failed API calls (default: 3)"
    )
    parser.add_argument(
        "--max_new_tokens",
        type=int,
        default=2048,
        help="Maximum number of new tokens for generation (default: 2048)"
    )
    
    args = parser.parse_args()

    if args.output_file is None:
        raise ValueError("Please provide an output file path using the -o or --output argument.")
    
    # Process the dataset
    process_dataset(
        args.dataset, 
        args.splits, 
        args.model_name, 
        args.output_file,
        max_workers=args.max_workers,
        max_retries=args.max_retries,
        max_new_tokens=args.max_new_tokens
    )


if __name__ == "__main__":
    main()