# Quick script to check your data
from datasets import load_dataset
from transformers import AutoTokenizer

tokenizer = AutoTokenizer.from_pretrained("meta-llama/CodeLlama-7b-Instruct-hf")
ds = load_dataset("ShethArihant/SeCodePLT-updated-CoT-v4", split="sft")

lengths = []
for ex in ds.select(range(min(100, len(ds)))):
    text = str(ex['prompt']) + str(ex.get('cot_steps', '')) + str(ex['completion'])
    tokens = tokenizer(text)['input_ids']
    lengths.append(len(tokens))

print(f"Max length: {max(lengths)}")
print(f"95th percentile: {sorted(lengths)[int(len(lengths)*0.95)]}")
print(f"Median: {sorted(lengths)[len(lengths)//2]}")