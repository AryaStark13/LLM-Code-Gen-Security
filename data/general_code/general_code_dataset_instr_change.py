from datasets import load_dataset, DatasetDict, Value, List, Features
import re

# Load the dataset
dataset = load_dataset("ShethArihant/General-Code-v1")

# Define mappings
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

def get_old_structured_instruction(lang: str) -> str:
    code_block_lang = LANG_TO_CODE_BLOCK.get(lang, lang)
    return (
        f"Important: Write your reasoning steps within <think> and </think> tags. And wrap your final code implementation within ```{code_block_lang} and ``` tags.\nExample format:\n<think>Your reasoning steps here...</think>\n```{code_block_lang}\nYour final code implementation here...\n```\n"
    )

NEW_INSTRUCTION = (
    "Important: Write your reasoning steps within <think> and </think> tags. "
    "And wrap your final code implementation within <code> and </code> tags.\n"
    "Example format:\n"
    "<think>Your reasoning steps here...</think>\n"
    "<code>\n"
    "Your final code implementation here...\n"
    "</code>"
)

# def modify_completion(content, lang):
#     code_block_lang = LANG_TO_CODE_BLOCK.get(lang, lang)
#     pattern = f"```{code_block_lang}\n(.*?)\n```"
#     replacement = "<code>\n\\1\n</code>"
#     return re.sub(pattern, replacement, content, flags=re.DOTALL)

def modify_completion(content, lang):
    # Build pattern to match any of the language code blocks
    all_langs = '|'.join(LANG_TO_CODE_BLOCK.values())
    pattern = f"```({all_langs})\n(.*?)\n```"
    matches = re.findall(pattern, content, flags=re.DOTALL)
    
    if matches:
        # Extract just the code content (second group in each match)
        code_blocks = [match[1] for match in matches]
        code_content = '\n\n'.join(code_blocks)
        return f"<code>\n{code_content}\n</code>".strip()
    else:
        # If no code block found, return original content
        return content

def modify_example(example, lang):
    old_instruction = get_old_structured_instruction(lang)
    
    if 'prompt' in example and isinstance(example['prompt'], list):
        for message in example['prompt']:
            if 'content' in message and old_instruction in message['content']:
                message['content'] = message['content'].replace(old_instruction, NEW_INSTRUCTION)
    
    if 'completion' in example and isinstance(example['completion'], list):
        for message in example['completion']:
            if 'content' in message:
                message['content'] = modify_completion(message['content'], lang)
    
    return example

# Process each split
new_dataset = {}
for split_name in dataset.keys():
    lang = split_name.split('_')[0]
    new_dataset[split_name] = dataset[split_name].map(
        lambda x: modify_example(x, lang),
        desc=f"Processing {split_name}"
    )

new_dataset = DatasetDict(new_dataset)

# Verify features
ref_features = Features({
    'task_id': Value('string'),
    'id': Value('string'),
    'CWE_ID': Value('int64'),
    'y_negative': Value('string'),
    'prompt': List({'content': Value('string'), 'role': Value('string')}),
    'cot_steps': Value('string'),
    'completion': List({'content': Value('string'), 'role': Value('string')})
})

for split in new_dataset.keys():
    assert new_dataset[split].features == ref_features

# Push to HuggingFace
new_dataset.push_to_hub("ShethArihant/General-Code-v1")

# # examples from the modified dataset
# for split in new_dataset.keys():
#     print(f"\nExamples from split: {split}")
#     for i in range(1):  # Print first 2 examples from each split
#         print(new_dataset[split][i])
#     break