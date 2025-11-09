from datasets import load_dataset
import pandas as pd
import sys
import os
import argparse
import json
from datetime import datetime
from tqdm import tqdm

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

def process_dataset(dataset, split, model_name, output_file=None):
    """Process the entire evaluation dataset with the specified model."""
    
    # Initialize the LLM client
    print(f"Initializing {model_name}...")
    llm_client = OpenAI(model_name=model_name)
    
    # Load the dataset
    print(f"Loading dataset: {dataset}")
    eval_dataset = load_dataset(dataset, split=split)
    
    total_examples = len(eval_dataset)
    print(f"Total examples to process: {total_examples}")
    
    results = []
    processed_count = 0
    
    # Process each example
    for idx, example in enumerate(tqdm(eval_dataset, desc="Processing examples")):
        try:
            # Extract the prompt (assuming it's in the format shown)
            prompt = example["prompt"][0]["content"] if isinstance(example["prompt"], list) else example["prompt"]
            
            # Get response from the model
            response, message = llm_client.send_message(prompt)
            
            # Build the result object
            result = {
                "index": idx,
                "task_id": example.get("task_id", ""),
                "id": example.get("id", idx),
                "CWE_ID": example.get("CWE_ID", None),
                "prompt": prompt,
                "ground_truth_cot": example.get("cot_steps", ""),
                "ground_truth_code": example.get("completion", "")[0]["content"],
                "ground_truth_full": example.get("cot_steps", "") + example.get("completion", "")[0]["content"],
                "y_negative": example.get("y_negative", ""),
                "output_with_tuning": message,
                "processed_at": processed_count + 1,
            }
            
            results.append(result)
            processed_count += 1
            
        except Exception as e:
            print(f"\nError processing example {idx}: {str(e)}")
            # Add a result with error information
            result = {
                "index": idx,
                "id": example.get("id", idx),
                "task_id": example.get("task_id", ""),
                "CWE_ID": example.get("CWE_ID", None),
                "prompt": example.get("prompt", ""),
                "ground_truth_cot": example.get("cot_steps", ""),
                "ground_truth_code": example.get("completion", "")[0]["content"],
                "ground_truth_full": example.get("cot_steps", "") + example.get("completion", "")[0]["content"],
                "y_negative": example.get("y_negative", ""),
                "output_with_tuning": f"ERROR: {str(e)}",
                "processed_at": processed_count + 1,
            }
            results.append(result)
            processed_count += 1
            continue
    
    # Create the final output object
    output_data = {
        "total_examples": total_examples,
        "processed_examples": processed_count,
        "model_name": model_name,
        "dataset": dataset,
        "timestamp": datetime.now().isoformat(),
        "results": results
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
    
    print(f"✓ Successfully processed {processed_count}/{total_examples} examples")
    print(f"✓ Results saved to: {output_file}")
    
    return output_data


def main():
    parser = argparse.ArgumentParser(
        description="Evaluate a language model on a Dataset of our format."
    )
    parser.add_argument(
        "--dataset",
        type=str,
        default="ShethArihant/CWEval-v1",
        help="Name of the dataset to use (default: ShethArihant/CWEval-v1)"
    )
    parser.add_argument(
        "--split",
        type=str,
        default="Python",
        help="Dataset split to use (default: Python)"
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
    
    args = parser.parse_args()

    if args.output_file is None:
        raise ValueError("Please provide an output file path using the -o or --output argument.")
    
    # Process the dataset
    process_dataset(args.dataset, args.split, args.model_name, args.output_file)


if __name__ == "__main__":
    main()