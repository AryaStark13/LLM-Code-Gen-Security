#!/usr/bin/env python3
"""
Script to convert SecCodePLT CoT SFT Results JSON to JSONL format.
Filters entries based on task IDs from a reference file.
"""

import json
import re
from pathlib import Path
from typing import Dict, List, Tuple
import sys
import argparse


def extract_code_from_output(output: str) -> str:
    """
    Extract code from the output string that's wrapped in <code>...</code> tags.
    
    Args:
        output: String containing code wrapped in XML-like tags
        
    Returns:
        Extracted code string, or empty string if no code found
    """
    if not output:
        return ""
    
    # Use regex to extract content between <code> and </code> tags
    pattern = r'<code>(.*?)</code>'
    match = re.search(pattern, output, re.DOTALL)
    
    if match:
        code = match.group(1).strip()
        return code
    
    return ""


def load_json_file(file_path: str) -> Dict:
    """Load and parse a JSON file."""
    with open(file_path, 'r', encoding='utf-8') as f:
        return json.load(f)


def convert_to_jsonl(
    results_file: str,
    task_ids_file: str,
    output_file: str
) -> Tuple[int, int, int]:
    """
    Convert JSON results to JSONL format with filtering.
    
    Args:
        results_file: Path to the SecCodePLT CoT SFT Results JSON file
        task_ids_file: Path to the task IDs reference JSON file
        output_file: Path for the output JSONL file
        
    Returns:
        Tuple of (total_entries, filtered_entries, entries_written)
    """
    # Load the results file
    print(f"Loading results from: {results_file}")
    results_data = load_json_file(results_file)
    
    # Load the task IDs reference file
    print(f"Loading task IDs from: {task_ids_file}")
    task_ids_data = load_json_file(task_ids_file)
    
    # Get the set of valid task IDs
    valid_task_ids = set(task_ids_data.keys())
    print(f"Found {len(valid_task_ids)} valid task IDs in reference file")
    
    # Process results
    results = results_data.get('results', [])
    total_entries = len(results)
    print(f"Found {total_entries} total entries in results file")
    
    entries_written = 0
    filtered_entries = 0
    entries_with_output = 0
    
    # Create output JSONL file
    with open(output_file, 'w', encoding='utf-8') as outfile:
        for result in results:
            task_id = result.get('id')
            
            # Check if task_id is in valid set
            if task_id not in valid_task_ids:
                filtered_entries += 1
                continue
            
            # Extract the generated code from output_with_tuning
            output_with_tuning = result.get('output_with_tuning', '')
            
            if not output_with_tuning:
                # Skip entries without output
                filtered_entries += 1
                continue
            
            entries_with_output += 1
            
            # Extract code from the <code> tags
            solution = extract_code_from_output(output_with_tuning)
            
            if not solution:
                print(f"Warning: Could not extract code for task_id {task_id}")
                filtered_entries += 1
                continue
            
            # Create JSONL entry
            entry = {
                "task_id": task_id,
                "solution": solution
            }
            
            # Write to file (one JSON object per line)
            outfile.write(json.dumps(entry) + '\n')
            entries_written += 1
    
    return total_entries, filtered_entries, entries_written


def main():
    """Main function to run the conversion."""
    # define arguments
    argparser = argparse.ArgumentParser(
        description="Convert SecCodePLT CoT SFT Results JSON to JSONL format."
    )
    argparser.add_argument(
        "--model_name",
        type=str,
        required=True,
        help="Model ID. Should be one of the folders in results/CoT_SFT/"
    )
    args = argparser.parse_args()
    model_name = args.model_name
    
    # Define file paths
    results_file = f"results/CoT_SFT/{model_name}/SecCodePLT_CoT_SFT_Results.json"
    task_ids_file = "data/SecCodePLT/SecCodePLT+_task-ids_func.json"
    output_file = f"results/CoT_SFT/{model_name}/SecCodePLT_CoT_SFT_Results.jsonl"
    
    print("=" * 60)
    print("SecCodePLT JSON to JSONL Conversion")
    print("=" * 60)
    
    try:
        total, filtered, written = convert_to_jsonl(
            results_file,
            task_ids_file,
            output_file
        )
        
        print("\n" + "=" * 60)
        print("CONVERSION STATISTICS")
        print("=" * 60)
        print(f"Total entries in results file:     {total}")
        print(f"Entries filtered out:              {filtered}")
        print(f"Entries written to JSONL:          {written}")
        print(f"Success rate:                      {written/total*100:.2f}%")
        print("=" * 60)
        print(f"\nOutput saved to: {output_file}")
        
    except FileNotFoundError as e:
        print(f"Error: Could not find file - {e}")
    except json.JSONDecodeError as e:
        print(f"Error: Invalid JSON format - {e}")
    except Exception as e:
        print(f"Error: {e}")


if __name__ == "__main__":
    main()