import pandas as pd
import os
import sys
from typing import Optional
from tqdm.auto import tqdm
import random

# Setup
random.seed(42)
tqdm.pandas()

# Import your LLM client
from utils.llms import OpenAI
llm_client = OpenAI(model_name="gpt-4o-2024-08-06")

from dotenv import load_dotenv
load_dotenv()


def get_structured_instruction() -> str:
    """Get the standard instruction format for structured prompts."""
    return (
        "Important: Write your reasoning steps within <think> and </think> tags. "
        "And wrap your final code implementation within <code> and </code> tags.\n"
        "Example format:\n"
        "<think>Your reasoning steps here...</think>\n"
        "<code>\n"
        "Your final code implementation here...\n"
        "</code>"
    )


def generate_structured_prompt(row: pd.Series) -> str:
    """Generate structured markdown format prompt from CSV row."""
    
    general_instructions = (
        "Think about the problem below carefully and step-by-step. "
        "Then implement the code that meets the requirements described."
    )
    
    func_signature = row['func_signature']
    task_description = row['task_description']
    cwe_id = row['CWE_ID']
    
    prompt_parts = []
    
    prompt_parts.append(f"### Instructions:\n{general_instructions}\n")
    # prompt_parts.append(f"**CWE ID:** {cwe_id}\n") # No security reminder!
    prompt_parts.append(f"**Function Signature:**\n<code>\n{func_signature}\n</code>\n")
    prompt_parts.append(f"**Description:**\n{task_description}\n")
    
    # Add standard instruction
    prompt_parts.append(get_structured_instruction() + "\n")
    
    return "\n".join(prompt_parts)


def generate_cot_prompt(X: str, y_positive: str) -> str:
    """
    Generate reasoning prompt for the larger model.
    
    Args:
        X: Input prompt
        y_positive: Safe code implementation
    
    Returns:
        CoT generation prompt
    """    
    cot_prompt = f"""{X}

Here is the safe code implementation:
{y_positive}

Let's reason through this security problem step by step. Explain your thought process to solve the above problem securely.
Do NOT provide any details of the actual code implementation in your reasoning.
Only include the reasoning, no other text.
Important: Be concise and to the point in your reasoning. Think step by step.
"""
    return cot_prompt


def generate_cot(X: str, y_positive: str) -> str:
    """Generate CoT response from larger model."""
    cot_prompt = generate_cot_prompt(X, y_positive)
    try:
        llm_response, llm_response_text = llm_client.send_message(cot_prompt)
        return llm_response_text
    except Exception as e:
        print(f"Error generating CoT: {e}")
        return ""


def process_row(row: pd.Series) -> pd.Series:
    """Process a single row to generate X, cot, y_positive, y_negative."""
    
    # Generate X (the prompt)
    X = generate_structured_prompt(row)
    
    # Generate y_positive (safe code wrapped in <code> tags)
    y_positive = f"<code>\n{row['safe_code']}\n</code>"
    
    # Generate y_negative (vulnerable code wrapped in <code> tags)
    y_negative = f"<code>\n{row['vulnerable_code']}\n</code>"
    
    # Generate CoT using larger model
    cot_text = generate_cot(X, y_positive)
    cot = f"<think>{cot_text}</think>\n" if cot_text else ""
    
    # Add new columns to row
    row['X'] = X
    row['cot'] = cot
    row['y_positive'] = y_positive
    row['y_negative'] = y_negative
    
    return row


def generate_dataset_with_cot(input_csv: str, output_csv: str):
    """
    Read CSV, generate CoT for each row, and save to new CSV.
    
    Args:
        input_csv: Path to input CSV file
        output_csv: Path to output CSV file
    """
    print(f"Reading dataset from {input_csv}...")
    df = pd.read_csv(input_csv)
    
    print(f"Total rows: {len(df)}")
    print(f"Columns: {df.columns.tolist()}")
    
    print("\nGenerating CoT for each example...")
    
    # Process each row with progress bar
    processed_rows = []
    for idx, row in tqdm(df.iterrows(), total=len(df), desc="Generating CoT"):
        try:
            processed_row = process_row(row)
            processed_rows.append(processed_row)
        except Exception as e:
            print(f"\nError processing row {idx}: {e}")
            # Add the row with empty CoT values
            row['X'] = ""
            row['cot'] = ""
            row['y_positive'] = ""
            row['y_negative'] = ""
            processed_rows.append(row)
    
    # Create new dataframe
    result_df = pd.DataFrame(processed_rows)
    
    print(f"\nSaving to {output_csv}...")
    result_df.to_csv(output_csv, index=False)
    
    print(f"✓ Saved {len(result_df)} entries to {output_csv}")
    
    return result_df


def preprocess_dataset_instruct(example):
    """Preprocess dataset for HuggingFace format."""
    prompt = [{"role": "user", "content": example["X"]}]
    completion = [{"role": "assistant", "content": example["y_positive"]}]
    
    return {
        "task_id": example["task_id"],
        "CWE_ID": example["CWE_ID"],
        "prompt": prompt,
        "cot_steps": example["cot"],
        "completion": completion,
        "y_negative": example["y_negative"]
    }


def convert_and_push_to_hf(csv_path: str, 
                          hf_dataset_name: str,
                          hf_token: Optional[str] = None):
    """
    Convert CSV to HuggingFace dataset and push to hub.
    
    Args:
        csv_path: Path to CSV file with CoT
        hf_dataset_name: Name for HuggingFace dataset (e.g., "username/dataset-name")
        hf_token: HuggingFace token (optional, will use cached token if not provided)
    """
    from datasets import Dataset, DatasetDict
    
    print(f"Loading CSV from {csv_path}...")
    df = pd.read_csv(csv_path)
    
    # Convert to HuggingFace Dataset
    print("Converting to HuggingFace Dataset format...")
    dataset = Dataset.from_pandas(df)
    
    # Apply preprocessing
    print("Applying preprocessing...")
    dataset = dataset.map(
        preprocess_dataset_instruct,
        desc="Preprocessing for HF format"
    )
    
    # Select only required columns
    required_columns = ['task_id', 'CWE_ID', 'y_negative', 'prompt', 'cot_steps', 'completion']
    dataset = dataset.select_columns(required_columns)
    
    print(f"\nDataset features: {dataset.features}")
    print(f"Dataset size: {len(dataset)}")
    print(f"\nFirst example:")
    print(dataset[0])
    
    # Push to hub
    if hf_token:
        print(f"\nPushing to HuggingFace Hub as {hf_dataset_name}...")
        dataset.push_to_hub(
            hf_dataset_name,
            token=hf_token,
            private=False  # Set to True if you want a private dataset
        )
        print(f"✓ Successfully pushed to https://huggingface.co/datasets/{hf_dataset_name}")
    else:
        print(f"\nNo HF token provided. Dataset prepared but not pushed.")
        print(f"To push later, use:")
        print(f"  dataset.push_to_hub('{hf_dataset_name}', token='your_token')")
    
    return dataset


def main():
    # Paths
    input_csv = "data/CWEval/data/cweval_dataset.csv"
    output_csv = "data/CWEval/data/cweval_dataset_with_cot.csv"
    hf_dataset_name = "ShethArihant/CWEval-v1"  # Change this
    
    # Step 1: Generate CoT and save to CSV
    print("="*60)
    print("Step 1: Generating Chain-of-Thought")
    print("="*60)
    df_with_cot = generate_dataset_with_cot(input_csv, output_csv)
    
    # Step 2: Convert and push to HuggingFace
    print("\n" + "="*60)
    print("Step 2: Converting to HuggingFace Dataset")
    print("="*60)
    
    # Get HF token from environment or set to None
    hf_token = os.environ.get('HF_TOKEN', None)
    
    dataset = convert_and_push_to_hf(
        output_csv, 
        hf_dataset_name,
        hf_token=hf_token
    )
    
    print("\n" + "="*60)
    print("✓ Complete!")
    print("="*60)
    print(f"CSV with CoT: {output_csv}")
    print(f"HuggingFace Dataset: {hf_dataset_name}")
    
    return df_with_cot, dataset


if __name__ == "__main__":
    df_with_cot, dataset = main()