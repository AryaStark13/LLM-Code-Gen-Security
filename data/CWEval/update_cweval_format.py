import os
import json
from datasets import load_dataset, DatasetDict, Dataset, Features, Value, List
from dotenv import load_dotenv
import argparse

load_dotenv()

# Define the reference schema that all datasets should match
REF_FEATURES = Features({
    'task_id': Value('string'),
    'id': Value('string'),
    'CWE_ID': Value('int64'),
    'y_negative': Value('string'),
    'prompt': List({'content': Value('string'), 'role': Value('string')}),
    'cot_steps': Value('string'),
    'completion': List({'content': Value('string'), 'role': Value('string')})
})

# Language to code block mapping
LANG_TO_CODE_BLOCK = {
    'c': 'c',
    'cpp': 'cpp',
    'go': 'go',
    'js': 'javascript',
    'py': 'python'
}

def get_old_structured_instruction(lang: str) -> str:
    """Get the OLD instruction format (markdown code blocks)."""
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

def get_new_structured_instruction(lang: str) -> str:
    """Get the NEW instruction format (XML code tags)."""
    return (
        "Important: Write your reasoning steps within <think> and </think> tags. "
        "And wrap your final code implementation within <code> and </code> tags.\n"
        "Example format:\n"
        "<think>Your reasoning steps here...</think>\n"
        "<code>\n"
        "Your final code implementation here...\n"
        "</code>"
    )

def extract_language_from_split(split_name: str) -> str:
    """Extract language from split name like 'core_c' or 'lang_py'"""
    parts = split_name.split('_')
    if len(parts) >= 2:
        lang = parts[-1]  # Last part is the language
        return lang
    return None

def update_prompt(prompt_messages: list, old_instruction: str, new_instruction: str) -> list:
    """Update prompt by replacing old instruction with new instruction."""
    updated_messages = []
    for message in prompt_messages:
        if message['role'] == 'user':
            content = message['content']
            # Replace the old instruction with the new one
            updated_content = content.replace(old_instruction, new_instruction)
            updated_messages.append({
                'role': message['role'],
                'content': updated_content
            })
        else:
            updated_messages.append(message)
    return updated_messages

def update_completion(completion_messages: list) -> list:
    """Wrap completion code in <code></code> tags."""
    updated_messages = []
    for message in completion_messages:
        if message['role'] == 'assistant':
            content = message['content']
            # Wrap the code in <code></code> tags
            updated_content = f"<code>\n{content}\n</code>"
            updated_messages.append({
                'role': message['role'],
                'content': updated_content
            })
        else:
            updated_messages.append(message)
    return updated_messages

def process_example(example, lang):
    """Process a single example to update prompt and completion."""
    # Get old and new instructions for this language
    old_instruction = get_old_structured_instruction(lang)
    new_instruction = get_new_structured_instruction(lang)
    
    # Update prompt
    updated_prompt = update_prompt(example['prompt'], old_instruction, new_instruction)
    
    # Update completion
    updated_completion = update_completion(example['completion'])
    
    # Return updated example with correct types
    return {
        'task_id': str(example['task_id']),
        'id': str(example['id']),
        'CWE_ID': int(example['CWE_ID']),
        'prompt': updated_prompt,
        'completion': updated_completion,
        'y_negative': str(example['y_negative']),
        'cot_steps': str(example['cot_steps'])
    }

def main():
    parser = argparse.ArgumentParser(description='Update CWEval dataset format from markdown to XML tags')
    parser.add_argument('--dataset', type=str, default='ShethArihant/CWEval-CoT',
                       help='HuggingFace dataset name')
    parser.add_argument('--output_dataset', type=str, default=None,
                       help='Output HuggingFace dataset name (default: same as input)')
    parser.add_argument('--push', action='store_true',
                       help='Push to HuggingFace Hub')
    parser.add_argument('--save_local', type=str, default=None,
                       help='Save to local directory (optional)')
    
    args = parser.parse_args()
    
    # Get HF token
    hf_token = os.environ.get('HF_TOKEN', None)
    if args.push and not hf_token:
        print("Error: HF_TOKEN not found in environment variables")
        return
    
    # Set output dataset name
    output_dataset_name = args.output_dataset if args.output_dataset else args.dataset
    
    print("="*60)
    print("CWEval Dataset Format Update")
    print("="*60)
    print(f"Input dataset: {args.dataset}")
    print(f"Output dataset: {output_dataset_name}")
    print("Changes:")
    print("  - Replace markdown code blocks (```language```) with <code></code> tags")
    print("  - Wrap completion code in <code></code> tags")
    print("  - Remove 'python' and 'train' splits")
    print("="*60)
    
    # Load dataset
    print("\nLoading dataset from HuggingFace...")
    dataset = load_dataset(args.dataset, token=hf_token)
    
    print(f"Original dataset splits: {list(dataset.keys())}")
    
    # Splits to remove
    splits_to_remove = ['python', 'train']
    
    # Process each split
    updated_dataset_dict = {}
    
    for split_name in dataset.keys():
        # Skip splits that should be removed
        if split_name in splits_to_remove:
            print(f"\n⊗ Skipping split: {split_name} (marked for removal)")
            continue
        
        print(f"\n{'='*60}")
        print(f"Processing split: {split_name}")
        print(f"{'='*60}")
        
        # Extract language from split name
        lang = extract_language_from_split(split_name)
        
        if lang not in LANG_TO_CODE_BLOCK:
            print(f"Warning: Could not determine language for split '{split_name}', skipping...")
            continue
        
        print(f"Language: {lang}")
        
        split_data = dataset[split_name]
        print(f"Examples in split: {len(split_data)}")
        
        # Process all examples in this split
        print("Updating prompts and completions...")
        updated_examples = [process_example(example, lang) for example in split_data]
        
        # Create new dataset from updated examples with schema validation
        print("Creating dataset with schema validation...")
        updated_split = Dataset.from_list(updated_examples, features=REF_FEATURES)
        
        # Validate features match reference
        print("\nSchema Validation:")
        print(f"  Expected features: {REF_FEATURES}")
        print(f"  Actual features: {updated_split.features}")
        
        if updated_split.features != REF_FEATURES:
            print(f"  ✗ WARNING: Features mismatch!")
            print(f"    Expected: {REF_FEATURES}")
            print(f"    Actual: {updated_split.features}")
            raise ValueError(f"Schema validation failed for split {split_name}")
        else:
            print(f"  ✓ Schema validation passed!")
        
        # Verify the update
        print("\nVerification:")
        print(f"  Original prompt sample (first 200 chars): {split_data[0]['prompt'][0]['content'][:200]}...")
        print(f"  Updated prompt sample (first 200 chars): {updated_split[0]['prompt'][0]['content'][:200]}...")
        print(f"  Original completion sample (first 100 chars): {split_data[0]['completion'][0]['content'][:100]}...")
        print(f"  Updated completion sample (first 100 chars): {updated_split[0]['completion'][0]['content'][:100]}...")
        
        # Check if changes were made
        old_has_markdown = '```' in split_data[0]['prompt'][0]['content']
        new_has_markdown = '```' in updated_split[0]['prompt'][0]['content']
        completion_has_code_tags = '<code>' in updated_split[0]['completion'][0]['content']
        
        print(f"  ✓ Markdown removed from prompt: {old_has_markdown} -> {new_has_markdown}")
        print(f"  ✓ Code tags added to completion: {completion_has_code_tags}")
        
        # Add to updated dataset dict
        updated_dataset_dict[split_name] = updated_split
        print(f"✓ Processed {len(updated_split)} examples")
    
    # Create final dataset
    print(f"\n{'='*60}")
    print("Creating updated dataset...")
    print(f"{'='*60}")
    
    final_dataset = DatasetDict(updated_dataset_dict)
    
    print(f"Final dataset splits: {list(final_dataset.keys())}")
    print(f"Removed splits: {splits_to_remove}")
    
    # Final schema validation for all splits
    print(f"\n{'='*60}")
    print("Final Schema Validation")
    print(f"{'='*60}")
    
    all_valid = True
    for split_name, split_data in final_dataset.items():
        print(f"\nValidating {split_name}:")
        print(f"  Examples: {len(split_data)}")
        print(f"  Features: {split_data.features}")
        
        if split_data.features != REF_FEATURES:
            print(f"  ✗ Schema mismatch!")
            all_valid = False
        else:
            print(f"  ✓ Schema valid!")
    
    if not all_valid:
        raise ValueError("Schema validation failed for one or more splits. Aborting.")
    
    print(f"\n✓ All splits passed schema validation!")
    
    # Save locally if requested
    if args.save_local:
        print(f"\nSaving to local directory: {args.save_local}")
        final_dataset.save_to_disk(args.save_local)
        print(f"✓ Saved locally to {args.save_local}")
    
    # Push to HuggingFace if requested
    if args.push:
        print(f"\nPushing to HuggingFace Hub: {output_dataset_name}")
        final_dataset.push_to_hub(
            output_dataset_name,
            token=hf_token,
            commit_message="Update format: Replace markdown code blocks with <code></code> tags, remove python and train splits"
        )
        print(f"✓ Successfully pushed to https://huggingface.co/datasets/{output_dataset_name}")
    else:
        print("\n⚠ Dataset not pushed to HuggingFace (use --push flag to push)")
    
    # Summary
    print(f"\n{'='*60}")
    print("Summary")
    print(f"{'='*60}")
    print(f"Processed splits: {len(final_dataset)}")
    for split_name, split_data in final_dataset.items():
        print(f"  - {split_name}: {len(split_data)} examples")
    print(f"Removed splits: {', '.join(splits_to_remove)}")
    print("\nChanges applied:")
    print("  ✓ Replaced markdown code blocks with <code></code> tags in prompts")
    print("  ✓ Wrapped completion code in <code></code> tags")
    print("  ✓ Removed outdated splits")
    print("\n✓ Complete!")


if __name__ == "__main__":
    main()