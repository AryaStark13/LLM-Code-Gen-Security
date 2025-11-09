import os
import json
import pandas as pd
from datasets import load_dataset
from dotenv import load_dotenv
from collections import defaultdict

# Load environment variables
load_dotenv()

# Configuration
TARGET_LANGUAGES = ['python', 'javascript', 'c', 'cpp', 'java', 'go', 'php', 'ruby', 'rust']
MAX_SAMPLES_PER_LANG = 500
MAX_QUERY_LENGTH = 2000
MAX_ANSWER_LENGTH = 2500
RESOURCES = ['wizardcoder', 'magicoder', 'evolinstruct', 'sharegpt']

# Output directory
OUTPUT_DIR = "data/general_code/data"
os.makedirs(OUTPUT_DIR, exist_ok=True)

def load_and_filter_dataset():
    """Load the dataset from HuggingFace"""
    print("Loading dataset from HuggingFace...")
    hf_token = os.getenv("HF_TOKEN")
    
    dataset = load_dataset(
        "m-a-p/CodeFeedback-Filtered-Instruction",
        split="train",
        token=hf_token
    )
    
    print(f"Total examples in dataset: {len(dataset)}")
    return dataset

def filter_by_length(example):
    """Check if example meets length requirements"""
    query_len = len(example['query']) if example['query'] else 0
    answer_len = len(example['answer']) if example['answer'] else 0
    
    return query_len <= MAX_QUERY_LENGTH and answer_len <= MAX_ANSWER_LENGTH

def stratified_sample_by_resource(df, n_samples):
    """Sample examples while maintaining resource distribution"""
    if len(df) <= n_samples:
        return df
    
    # Calculate proportional samples from each resource
    resource_counts = df['resource'].value_counts()
    samples_per_resource = {}
    
    for resource in resource_counts.index:
        proportion = resource_counts[resource] / len(df)
        samples_per_resource[resource] = max(1, int(n_samples * proportion))
    
    # Adjust to ensure we get exactly n_samples
    total_allocated = sum(samples_per_resource.values())
    if total_allocated < n_samples:
        # Add remaining samples to the largest resource
        largest_resource = resource_counts.index[0]
        samples_per_resource[largest_resource] += (n_samples - total_allocated)
    elif total_allocated > n_samples:
        # Remove excess from largest resource
        largest_resource = resource_counts.index[0]
        samples_per_resource[largest_resource] -= (total_allocated - n_samples)
    
    # Sample from each resource
    sampled_dfs = []
    for resource, n in samples_per_resource.items():
        resource_df = df[df['resource'] == resource]
        if len(resource_df) >= n:
            sampled = resource_df.sample(n=n, random_state=42)
        else:
            sampled = resource_df
        sampled_dfs.append(sampled)
    
    result = pd.concat(sampled_dfs, ignore_index=True)
    
    # If we still don't have enough samples, add more randomly
    if len(result) < n_samples:
        remaining = n_samples - len(result)
        remaining_df = df[~df.index.isin(result.index)]
        if len(remaining_df) > 0:
            extra = remaining_df.sample(n=min(remaining, len(remaining_df)), random_state=42)
            result = pd.concat([result, extra], ignore_index=True)
    
    return result.head(n_samples)

def transform_to_output_format(df, lang):
    """Transform the dataframe to the required output format"""
    output_data = []
    
    for idx, row in df.iterrows():
        # Create prompt in the required format
        prompt = json.dumps([{"role": "user", "content": row['query']}])
        
        # Create completion in the required format
        completion = json.dumps([{"role": "assistant", "content": row['answer']}])
        
        # Create the output record
        record = {
            'task_id': f"general_code_{lang}_{idx}",
            'id': str(idx),
            'CWE_ID': -1,
            'prompt': prompt,
            'completion': completion,
            'y_negative': '',
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
    
    # Filter by length
    lang_data = lang_data.filter(filter_by_length)
    print(f"Examples after length filtering: {len(lang_data)}")
    
    if len(lang_data) == 0:
        print(f"No examples found for {lang} after filtering!")
        return None
    
    # Convert to pandas for easier manipulation
    df = pd.DataFrame(lang_data)
    
    # Show resource distribution before sampling
    print("\nResource distribution before sampling:")
    print(df['resource'].value_counts())
    
    # Determine number of samples
    n_samples = min(MAX_SAMPLES_PER_LANG, len(df))
    print(f"\nSampling {n_samples} examples...")
    
    # Stratified sampling by resource
    sampled_df = stratified_sample_by_resource(df, n_samples)
    
    print("\nResource distribution after sampling:")
    print(sampled_df['resource'].value_counts())
    
    # Transform to output format
    output_df = transform_to_output_format(sampled_df, lang)
    
    # Save to CSV
    output_path = os.path.join(OUTPUT_DIR, f"general_code_dataset_{lang}.csv")
    output_df.to_csv(output_path, index=False)
    print(f"\n✓ Saved to: {output_path}")
    print(f"  Total examples: {len(output_df)}")
    
    return output_df

def main():
    """Main processing function"""
    print("Starting dataset preprocessing...")
    print(f"Target languages: {', '.join(TARGET_LANGUAGES)}")
    print(f"Max samples per language: {MAX_SAMPLES_PER_LANG}")
    print(f"Max query length: {MAX_QUERY_LENGTH} chars")
    print(f"Max answer length: {MAX_ANSWER_LENGTH} chars")
    
    # Load dataset
    dataset = load_and_filter_dataset()
    
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
    
    print("\n✓ Dataset preprocessing complete!")

if __name__ == "__main__":
    main()