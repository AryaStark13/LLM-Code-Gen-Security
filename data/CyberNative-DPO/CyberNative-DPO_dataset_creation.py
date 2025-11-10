import os
import json
import pandas as pd
from datasets import load_dataset
from dotenv import load_dotenv

# Load environment variables
load_dotenv()

# Configuration
TARGET_LANGUAGES = ['python', 'java', 'javascript', 'php', 'ruby', 'go']
MAX_QUERY_LENGTH = 2000
MAX_ANSWER_LENGTH = 2500

# Output directory
OUTPUT_DIR = "data/CyberNative-DPO/data"
os.makedirs(OUTPUT_DIR, exist_ok=True)

def load_and_filter_dataset():
    """Load the dataset from HuggingFace"""
    print("Loading dataset from HuggingFace...")
    hf_token = os.getenv("HF_TOKEN")
    
    dataset = load_dataset(
        "CyberNative/Code_Vulnerability_Security_DPO",
        split="train",
        token=hf_token
    )
    
    print(f"Total examples in dataset: {len(dataset)}")
    return dataset

def filter_by_length(example):
    """Check if example meets length requirements"""
    query_len = len(example['question']) if example['question'] else 0
    answer_len = len(example['chosen']) if example['chosen'] else 0
    
    return query_len <= MAX_QUERY_LENGTH and answer_len <= MAX_ANSWER_LENGTH

def transform_to_output_format(df, lang):
    """Transform the dataframe to the required output format"""
    output_data = []
    
    for idx, row in df.iterrows():
        # Create prompt in the required format
        prompt = json.dumps([{"role": "user", "content": row['question']}])
        
        # Create completion in the required format (chosen = safe code)
        completion = json.dumps([{"role": "assistant", "content": row['chosen']}])
        
        # y_negative is the rejected (unsafe) code
        y_negative = f"\n{row['rejected']}\n" if row['rejected'] else ""
        
        # Create the output record
        record = {
            'task_id': f"CyberNative-DPO_{lang}_{idx}",
            'id': str(idx),
            'CWE_ID': -1,
            'prompt': prompt,
            'completion': completion,
            'y_negative': y_negative,
            'cot_steps': ''
        }
        
        output_data.append(record)
    
    return pd.DataFrame(output_data)

def process_language(dataset, lang):
    """Process data for a specific language"""
    print(f"\n{'='*60}")
    print(f"Processing language: {lang}")
    print(f"{'='*60}")
    
    # Filter by language
    lang_data = dataset.filter(lambda x: x['lang'] == lang)
    print(f"Total examples for {lang}: {len(lang_data)}")
    
    if len(lang_data) == 0:
        print(f"No examples found for {lang}!")
        return None
    
    # Filter by length
    print("Applying length filters...")
    lang_data_filtered = lang_data.filter(filter_by_length)
    print(f"Examples after length filtering: {len(lang_data_filtered)}")
    
    if len(lang_data_filtered) == 0:
        print(f"No examples remain for {lang} after filtering!")
        return None
    
    # Convert to pandas for easier manipulation
    df = pd.DataFrame(lang_data_filtered)
    
    # Show statistics
    print(f"\nStatistics:")
    print(f"  - Total examples: {len(df)}")
    print(f"  - Has vulnerability info: {df['vulnerability'].notna().sum()}")
    print(f"  - Has rejected code: {df['rejected'].notna().sum()}")
    
    # Transform to output format
    output_df = transform_to_output_format(df, lang)
    
    # Save to CSV
    output_path = os.path.join(OUTPUT_DIR, f"CyberNative-DPO_dataset_{lang}.csv")
    output_df.to_csv(output_path, index=False)
    print(f"\n✓ Saved to: {output_path}")
    print(f"  Total examples: {len(output_df)}")
    
    return output_df

def main():
    """Main processing function"""
    print("Starting CyberNative-DPO dataset preprocessing...")
    print(f"Target languages: {', '.join(TARGET_LANGUAGES)}")
    print(f"Max query length: {MAX_QUERY_LENGTH} chars")
    print(f"Max answer length: {MAX_ANSWER_LENGTH} chars")
    
    # Load dataset
    dataset = load_and_filter_dataset()
    
    # Show language distribution
    print("\n" + "="*60)
    print("Language distribution in dataset:")
    print("="*60)
    df_full = pd.DataFrame(dataset)
    lang_counts = df_full['lang'].value_counts()
    for lang in TARGET_LANGUAGES:
        count = lang_counts.get(lang, 0)
        print(f"{lang:15} {count:5} examples")
    
    # Process each language
    results = {}
    for lang in TARGET_LANGUAGES:
        result = process_language(dataset, lang)
        if result is not None:
            results[lang] = result
    
    # Summary
    print("\n" + "="*60)
    print("SUMMARY")
    print("="*60)
    for lang in TARGET_LANGUAGES:
        if lang in results:
            print(f"{lang:15} {len(results[lang]):5} examples")
        else:
            print(f"{lang:15}     0 examples")
    
    print("\n✓ CyberNative-DPO dataset preprocessing complete!")
    print(f"Output directory: {OUTPUT_DIR}")

if __name__ == "__main__":
    main()