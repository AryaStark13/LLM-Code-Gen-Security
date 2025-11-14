# from datasets import load_dataset, DatasetDict, Value, List, Features
# import re

# # Load the dataset
# dataset = load_dataset("ShethArihant/CyberNative-DPO-v1")

# # Define mappings
# LANG_TO_CODE_BLOCK = {
#     'python': 'python',
#     'java': 'java',
#     'javascript': 'javascript',
#     'php': 'php',
#     'ruby': 'ruby',
#     'go': 'go'
# }

# def get_old_structured_instruction(lang: str) -> str:
#     code_block_lang = LANG_TO_CODE_BLOCK.get(lang, lang)
#     return (
#         "Important: Write your reasoning steps within <think> and </think> tags. "
#         f"And wrap your final code implementation within ```{code_block_lang} and ``` tags.\n"
#         "Example format:\n"
#         "<think>Your security reasoning steps here...</think>\n"
#         f"```{code_block_lang}\n"
#         "Your final secure code implementation here...\n"
#         "```"
#     )

# NEW_INSTRUCTION = (
#     "Important: Write your reasoning steps within <think> and </think> tags. "
#     "And wrap your final code implementation within <code> and </code> tags.\n"
#     "Example format:\n"
#     "<think>Your reasoning steps here...</think>\n"
#     "<code>\n"
#     "Your final code implementation here...\n"
#     "</code>"
# )

# def modify_prompt(content, lang):
#     """modify the prompt content
#     security_instructions = (
#         "Think about the security implications of this problem carefully and step-by-step. "
#         "Then implement secure code that addresses the vulnerability and meets the requirements described. "
#         "Add imports if necessary and follow security best practices."
#     )
#     these instructions are to be removed. they were 
#     """

# def modify_completion(content, lang):
#     code_block_lang = LANG_TO_CODE_BLOCK.get(lang, lang)
#     pattern = f"```{code_block_lang}\n(.*?)\n```"
#     replacement = "<code>\n\\1\n</code>"
#     return re.sub(pattern, replacement, content, flags=re.DOTALL)

# def modify_y_negative(content, lang):
#     code_block_lang = LANG_TO_CODE_BLOCK.get(lang, lang)
#     pattern = f"```{code_block_lang}\n(.*?)\n```"
#     replacement = "<code>\n\\1\n</code>"
#     return re.sub(pattern, replacement, content, flags=re.DOTALL).strip()

# def modify_example(example, lang):
#     old_instruction = get_old_structured_instruction(lang)
    
#     if 'prompt' in example and isinstance(example['prompt'], list):
#         for message in example['prompt']:
#             if 'content' in message and old_instruction in message['content']:
#                 message['content'] = message['content'].replace(old_instruction, NEW_INSTRUCTION)
    
#     if 'completion' in example and isinstance(example['completion'], list):
#         for message in example['completion']:
#             if 'content' in message:
#                 message['content'] = modify_completion(message['content'], lang)

#     if 'y_negative' in example and isinstance(example['y_negative'], str):
#         example['y_negative'] = modify_y_negative(example['y_negative'], lang)
    
#     return example

# # Process each split
# new_dataset = {}
# for split_name in dataset.keys():
#     lang = split_name.split('_')[0]
#     new_dataset[split_name] = dataset[split_name].map(
#         lambda x: modify_example(x, lang),
#         desc=f"Processing {split_name}"
#     )

# new_dataset = DatasetDict(new_dataset)

# # Verify features
# ref_features = Features({
#     'task_id': Value('string'),
#     'id': Value('string'),
#     'CWE_ID': Value('int64'),
#     'y_negative': Value('string'),
#     'prompt': List({'content': Value('string'), 'role': Value('string')}),
#     'cot_steps': Value('string'),
#     'completion': List({'content': Value('string'), 'role': Value('string')})
# })

# for split in new_dataset.keys():
#     assert new_dataset[split].features == ref_features

# # Push to HuggingFace
# # new_dataset.push_to_hub("ShethArihant/CyberNative-DPO-v1")

# # examples from the modified dataset
# for split in new_dataset.keys():
#     print(f"\nExamples from split: {split}")
#     for i in range(2):  # Print first 2 examples from each split
#         print(new_dataset[split][i])
#     break


from datasets import load_dataset, DatasetDict, Value, List, Features

# Load the dataset
dataset = load_dataset("ShethArihant/CyberNative-DPO-v1")

SECURITY_INSTRUCTIONS_TEXT = (
    "### Security Instructions:\n"
    "Think about the security implications of this problem carefully and step-by-step. "
    "Then implement secure code that addresses the vulnerability and meets the requirements described. "
    "Add imports if necessary and follow security best practices.\n\n"
)

def remove_security_instructions(example):
    """This function removes the security instructions from the prompt.
    And also performs .strip() on y_negative to clean up any leading/trailing whitespace."""
    if 'prompt' in example and isinstance(example['prompt'], list):
        for message in example['prompt']:
            if 'content' in message and SECURITY_INSTRUCTIONS_TEXT in message['content']:
                message['content'] = message['content'].replace(SECURITY_INSTRUCTIONS_TEXT, '')

    if 'y_negative' in example and isinstance(example['y_negative'], str):
        example['y_negative'] = example['y_negative'].strip()
    return example

# Process each split
new_dataset = {}
for split_name in dataset.keys():
    new_dataset[split_name] = dataset[split_name].map(
        remove_security_instructions,
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
new_dataset.push_to_hub("ShethArihant/CyberNative-DPO-v1")