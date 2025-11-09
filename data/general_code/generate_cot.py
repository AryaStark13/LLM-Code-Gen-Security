import pandas as pd
import os
import sys
import json
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


# Language to code block mapping
LANG_TO_CODE_BLOCK = {
    'python': 'python',
    'javascript': 'javascript',
    'c': 'c',
    'cpp': 'cpp',
    'java': 'java',
    'go': 'go',
    'php': 'php',
    'ruby': 'ruby',
    'rust': 'rust'
}


def get_structured_instruction(lang: str) -> str:
    """Get the standard instruction format for structured prompts."""
    code_block_lang = LANG_TO_CODE_BLOCK.get(lang, lang)
    return (
        "Important: Write your reasoning steps within <think> and </think> tags. "
        f"And wrap your final code implementation within ```{code_block_lang} and ``` tags.\n"
        "Example format:\n"
        "<think>Your reasoning steps here...</think>\n"
        f"```{code_block_lang}\n"
        "Your final code implementation here...\n"
        "```"
    )


def generate_structured_prompt(row: pd.Series, lang: str) -> str:
    """Generate structured markdown format prompt from CSV row."""
    
    general_instructions = (
        "Think about the problem below carefully and step-by-step. "
        "Then implement the code that meets the requirements described. "
        "Add imports if necessary."
    )
    
    # Parse the prompt from JSON
    prompt_data = json.loads(row['prompt'])
    actual_prompt = prompt_data[0]['content']
    
    prompt_parts = []
    
    prompt_parts.append(f"### Instructions:\n{general_instructions}\n")
    prompt_parts.append(f"**Problem:**\n{actual_prompt}\n")
    
    # Add standard instruction
    prompt_parts.append(get_structured_instruction(lang) + "\n")
    
    return "\n".join(prompt_parts)


def generate_cot_prompt(X: str, y_positive: str) -> str:
    """
    Generate reasoning prompt for the larger model.
    
    Args:
        X: Input prompt
        y_positive: Code implementation
    
    Returns:
        CoT generation prompt
    """
    cot_prompt = f"""{X}

Here is the code implementation:
{y_positive}

Let's reason through this problem step by step. Explain your thought process to solve the above problem.
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


def process_row(row: pd.Series, lang: str) -> pd.Series:
    """Process a single row to generate X and cot_steps."""
    
    # Generate X (the prompt)
    X = generate_structured_prompt(row, lang)
    
    # Parse completion to get code
    completion_data = json.loads(row['completion'])
    completion_content = completion_data[0]['content']
    # code_content = completion_data[0]['content']
    
    # # Wrap code in markdown code block
    # code_block_lang = LANG_TO_CODE_BLOCK.get(lang, lang)
    # y_positive = f"```{code_block_lang}\n{code_content}\n```"
    
    # Generate CoT using larger model
    cot_text = generate_cot(X, completion_content)
    cot_steps = f"<think>{cot_text}</think>\n" if cot_text else ""
    
    # Update the row
    row['prompt'] = json.dumps([{"role": "user", "content": X}])
    # row['completion'] = json.dumps([{"role": "assistant", "content": f"{cot_steps}{y_positive}"}]) # does not change completion (handled in train script)
    row['cot_steps'] = cot_steps
    
    return row


def generate_dataset_with_cot(input_csv: str, output_csv: str, lang: str):
    """
    Read CSV, generate CoT for each row, and save to new CSV.
    
    Args:
        input_csv: Path to input CSV file
        output_csv: Path to output CSV file
        lang: Programming language
    """
    print(f"\n{'='*60}")
    print(f"Processing {lang.upper()} dataset")
    print(f"{'='*60}")
    print(f"Reading dataset from {input_csv}...")
    df = pd.read_csv(input_csv)
    
    print(f"Total rows: {len(df)}")
    print(f"Columns: {df.columns.tolist()}")
    
    print(f"\nGenerating CoT for each example...")
    
    # Process each row with progress bar
    processed_rows = []
    for idx, row in tqdm(df.iterrows(), total=len(df), desc=f"Generating CoT for {lang}"):
        try:
            processed_row = process_row(row, lang)
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
    
    return result_df


def create_hf_dataset_dict(cot_data_dir: str, languages: list) -> dict:
    """
    Create a HuggingFace DatasetDict from language-specific CSV files.
    
    Args:
        cot_data_dir: Directory containing CSV files with CoT
        languages: List of programming languages
    
    Returns:
        DatasetDict with language splits
    """
    from datasets import Dataset, DatasetDict
    
    print("\n" + "="*60)
    print("Creating HuggingFace DatasetDict")
    print("="*60)
    
    dataset_dict = {}
    
    for lang in languages:
        csv_path = os.path.join(cot_data_dir, f"general_code_dataset_{lang}_with_cot.csv")
        
        if not os.path.exists(csv_path):
            print(f"Warning: {csv_path} not found, skipping {lang}")
            continue
        
        print(f"\nLoading {lang} dataset from {csv_path}...")
        df = pd.read_csv(csv_path)
        
        # Convert to HuggingFace Dataset
        dataset = Dataset.from_pandas(df)
        
        # Ensure required columns are present
        required_columns = ['task_id', 'id', 'CWE_ID', 'prompt', 'completion', 'y_negative', 'cot_steps']
        available_columns = [col for col in required_columns if col in dataset.column_names]
        dataset = dataset.select_columns(available_columns)
        
        dataset_dict[lang] = dataset
        print(f"✓ Added {lang} with {len(dataset)} examples")
    
    # Create DatasetDict
    hf_dataset = DatasetDict(dataset_dict)
    
    print(f"\n{'='*60}")
    print("DatasetDict Summary:")
    print(f"{'='*60}")
    for lang, dataset in hf_dataset.items():
        print(f"{lang:15} {len(dataset):5} examples")
    
    return hf_dataset


def push_to_hf(dataset_dict, hf_dataset_name: str, hf_token: Optional[str] = None):
    """
    Push DatasetDict to HuggingFace Hub.
    
    Args:
        dataset_dict: HuggingFace DatasetDict
        hf_dataset_name: Name for HuggingFace dataset
        hf_token: HuggingFace token
    """
    if hf_token:
        print(f"\nPushing to HuggingFace Hub as {hf_dataset_name}...")
        dataset_dict.push_to_hub(
            hf_dataset_name,
            token=hf_token,
            private=False,
            commit_message="Initial upload: General code dataset with CoT reasoning"
        )
        print(f"✓ Successfully pushed to https://huggingface.co/datasets/{hf_dataset_name}")
    else:
        print(f"\nNo HF token provided. Dataset prepared but not pushed.")
        print(f"To push later, use:")
        print(f"  dataset_dict.push_to_hub('{hf_dataset_name}', token='your_token')")


def main():
    # Configuration
    languages = ['python', 'javascript', 'c', 'cpp', 'java', 'go', 'php', 'ruby', 'rust']
    input_data_dir = "data/general_code/data"
    output_data_dir = "data/general_code/data/cot_data"
    hf_dataset_name = "ShethArihant/General-Code-v1"
    
    # Create output directory
    os.makedirs(output_data_dir, exist_ok=True)
    
    # Step 1: Generate CoT for each language dataset
    print("="*60)
    print("Step 1: Generating Chain-of-Thought for all languages")
    print("="*60)
    
    for lang in languages:
        input_csv = os.path.join(input_data_dir, f"general_code_dataset_{lang}.csv")
        output_csv = os.path.join(output_data_dir, f"general_code_dataset_{lang}_with_cot.csv")
        
        if not os.path.exists(input_csv):
            print(f"\nWarning: {input_csv} not found, skipping {lang}")
            continue
        
        try:
            df_with_cot = generate_dataset_with_cot(input_csv, output_csv, lang)
        except Exception as e:
            print(f"\nError processing {lang}: {e}")
            continue
    
    # Step 2: Create HuggingFace DatasetDict
    print("\n" + "="*60)
    print("Step 2: Creating HuggingFace DatasetDict")
    print("="*60)
    
    dataset_dict = create_hf_dataset_dict(output_data_dir, languages)
    
    # Step 3: Push to HuggingFace
    print("\n" + "="*60)
    print("Step 3: Pushing to HuggingFace Hub")
    print("="*60)
    
    hf_token = os.environ.get('HF_TOKEN', None)
    push_to_hf(dataset_dict, hf_dataset_name, hf_token)
    
    # Final summary
    print("\n" + "="*60)
    print("✓ Complete!")
    print("="*60)
    print(f"CSV files with CoT: {output_data_dir}")
    print(f"HuggingFace Dataset: {hf_dataset_name}")
    print("\nDataset splits:")
    for lang in dataset_dict.keys():
        print(f"  - {lang}: {len(dataset_dict[lang])} examples")
    
    return dataset_dict


if __name__ == "__main__":
    dataset_dict = main()