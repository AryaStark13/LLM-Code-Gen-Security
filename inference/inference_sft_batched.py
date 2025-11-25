# import unsloth # type: ignore[import]
# from unsloth import FastLanguageModel # type: ignore[import]
import argparse
import torch
from transformers import AutoModelForCausalLM, AutoTokenizer
from peft import PeftModel
from transformers import StoppingCriteria, StoppingCriteriaList
from datasets import load_dataset
import json
from tabulate import tabulate
import os
from typing import List
from tqdm import tqdm

device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

class CustomStopCriteria(StoppingCriteria):
    def __init__(self, stop_sequence_ids):
        self.stop_sequence_ids = stop_sequence_ids

    def __call__(self, input_ids: torch.LongTensor, scores: torch.FloatTensor, **kwargs) -> bool:
        # Check if all sequences in the batch have the stop sequence
        for i in range(input_ids.shape[0]):
            if input_ids[i, -len(self.stop_sequence_ids):].tolist() != self.stop_sequence_ids:
                return False
        return True

def load_base_model_and_tokenizer(base_model_path):
    print("Base Model Path:", base_model_path)
    is_local = os.path.isdir(base_model_path)
    base_model = AutoModelForCausalLM.from_pretrained(
        base_model_path,
        # torch_dtype=torch.float16,
        torch_dtype=torch.bfloat16,
        device_map="auto",
        trust_remote_code=True,
        local_files_only=is_local
    )

    tokenizer = AutoTokenizer.from_pretrained(base_model_path, trust_remote_code=True, local_files_only=is_local)
    
    # Set pad token if not already set
    if tokenizer.pad_token is None:
        tokenizer.pad_token = tokenizer.eos_token
        tokenizer.pad_token_id = tokenizer.eos_token_id
    
    # Enable padding on the left for generation
    tokenizer.padding_side = "left"
    
    return base_model, tokenizer

def load_sft_lora_adapter(base_model, sft_lora_adapter_path):
    print("SFT LoRA Adapter Path:", sft_lora_adapter_path)
    is_local = os.path.isdir(sft_lora_adapter_path)
    sft_lora_model = PeftModel.from_pretrained(
        base_model,
        sft_lora_adapter_path,
        local_files_only=is_local
    )
    return sft_lora_model

def merge_sft_lora_adapter(sft_lora_model):
    merged_model = sft_lora_model.merge_and_unload()
    return merged_model

def generate_output_batch(prompts: List[str], model, tokenizer, stopping_criteria, max_new_tokens=2048) -> List[str]:
    """Generate outputs for a batch of prompts"""
    # Tokenize all prompts with padding
    inputs = tokenizer(
        prompts, 
        return_tensors="pt", 
        padding=True, 
        truncation=True,
        max_length=4096  # Adjust based on your model's context length
    ).to(device)

    with torch.no_grad():
        outputs = model.generate(
            **inputs, 
            max_new_tokens=max_new_tokens,
            stopping_criteria=stopping_criteria,
            pad_token_id=tokenizer.pad_token_id
        )

    # Decode each output, removing the input prompt
    generated_texts = []
    for i, output in enumerate(outputs):
        # Get the length of the input for this specific example
        input_length = inputs["input_ids"][i].shape[0]
        # Decode only the generated part
        generated_text = tokenizer.decode(output[input_length:], skip_special_tokens=True)
        generated_texts.append(generated_text)
    
    return generated_texts

def run_full_evaluation(model, tokenizer, stopping_criteria, eval_dataset, split_name, 
                       batch_size=1, output_file="evaluation_results.json"):
    """Run evaluation on the full dataset with batching and save results as JSON"""
    # make sure output file directory exists
    output_dir = os.path.dirname(output_file)
    if output_dir:
        os.makedirs(output_dir, exist_ok=True)

    results = []
    num_examples = len(eval_dataset)
    
    # Process in batches
    for batch_start in tqdm(range(0, num_examples, batch_size), 
                           desc=f"Processing {split_name}"):
        batch_end = min(batch_start + batch_size, num_examples)
        batch_indices = range(batch_start, batch_end)
        
        # Collect batch data
        batch_examples = [eval_dataset[i] for i in batch_indices]
        batch_prompts = [example["prompt"][0]["content"] for example in batch_examples]
        
        # Generate outputs for the batch
        try:
            generated_outputs = generate_output_batch(
                batch_prompts, model, tokenizer, stopping_criteria
            )
        except Exception as e:
            print(f"\nError generating outputs for batch {batch_start}-{batch_end}: {e}")
            generated_outputs = [""] * len(batch_prompts)
        
        # Create result entries for each example in the batch
        for idx, (example, generated_output) in enumerate(zip(batch_examples, generated_outputs)):
            global_idx = batch_start + idx
            cot_steps = ""
            
            result = {
                "split": split_name,
                "index": global_idx,
                "task_id": example.get("task_id", ""),
                "id": example.get("id", ""),
                "CWE_ID": example.get("CWE_ID", ""),
                "prompt": example["prompt"][0]["content"],
                # "ground_truth_cot": example.get("cot_steps", ""),
                "ground_truth_cot": cot_steps,
                "ground_truth_code": example.get("completion", "")[0]["content"] if example.get("completion") else "",
                # "ground_truth_full": example.get("cot_steps", "") + (example.get("completion", "")[0]["content"] if example.get("completion") else ""),
                "ground_truth_full": cot_steps + (example.get("completion", "")[0]["content"] if example.get("completion") else ""),
                "y_negative": example.get("y_negative", ""),
                "output_without_tuning": "",
                "output_with_tuning": generated_output,
                "processed_at": global_idx + 1,
                "has_non_tuned_output": False
            }
            
            results.append(result)
        
        # Save intermediate results every 10 batches
        if (batch_start // batch_size + 1) % 10 == 0:
            intermediate_output = {
                "total_examples": num_examples,
                "processed_examples": len(results),
                "results": results
            }
            intermediate_file = f"{output_file}.intermediate"
            with open(intermediate_file, 'w') as f:
                json.dump(intermediate_output, f, indent=2)
    
    return results


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--dataset", type=str, required=True, 
                       help="Valid HF dataset path. Should be in the pre-processed format.")
    parser.add_argument("--splits", nargs='+', type=str, required=True,
                       help="Dataset splits to use for evaluation (e.g., core_c core_py)")
    parser.add_argument("--base_model", type=str, default="deepseek-ai/deepseek-coder-7b-instruct-v1.5")
    parser.add_argument("--sft_lora_adapter", type=str, 
                       default="ShethArihant/deepseek-coder-7b-instruct-v1.5_sft_2-epochs")
    parser.add_argument("--batch_size", type=int, default=1,
                       help="Batch size for inference (default: 1)")
    parser.add_argument("--max_new_tokens", type=int, default=2048,
                       help="Maximum number of new tokens to generate (default: 2048)")
    parser.add_argument("--run_eval", action="store_true", 
                       help="Run evaluation on the full dataset")
    parser.add_argument("--eval_output", type=str, 
                       help="Output file for evaluation results")

    args = parser.parse_args()

    # Validate arguments
    if args.run_eval and not args.eval_output:
        parser.error("--eval_output is required when --run_eval is specified")

    # Load base model
    print("Loading base model and tokenizer...")
    base_model, tokenizer = load_base_model_and_tokenizer(args.base_model)

    # Load SFT LoRA adapter
    print("Loading SFT LoRA adapter...")
    sft_lora_model = load_sft_lora_adapter(base_model, args.sft_lora_adapter)

    # Merge SFT LoRA adapter into base model
    print("Merging SFT LoRA adapter...")
    sft_merged_model = merge_sft_lora_adapter(sft_lora_model)

    stop_sequence_ids = tokenizer.encode("</code>", add_special_tokens=False)
    stopping_criteria = StoppingCriteriaList([CustomStopCriteria(stop_sequence_ids)])
    
    print("Model is ready for inference with SFT LoRA adapter.")
    print(f"Total parameters in the model: {sum(p.numel() for p in sft_merged_model.parameters()):,}")
    print(f"Batch size: {args.batch_size}")
    print(f"Max new tokens: {args.max_new_tokens}")

    if args.run_eval:
        # Collect all results across splits
        all_results = []
        
        # Process each split
        for split in args.splits:
            print(f"\n{'='*60}")
            print(f"Processing split: {split}")
            print(f"{'='*60}")
            
            # Load evaluation dataset for this split
            eval_dataset = load_dataset(
                args.dataset,
                split=split
            )
            
            print(f"Dataset size: {len(eval_dataset)} examples")
            print(f"Processing in batches of {args.batch_size}...")
            
            # Run evaluation and collect results
            split_results = run_full_evaluation(
                sft_merged_model, tokenizer, stopping_criteria, 
                eval_dataset, split, 
                batch_size=args.batch_size,
                output_file=args.eval_output
            )
            all_results.extend(split_results)
            
            print(f"Completed {split}: {len(split_results)} examples")
        
        # Save all results to a single JSON file
        output_dir = os.path.dirname(args.eval_output)
        if output_dir:
            os.makedirs(output_dir, exist_ok=True)
        
        final_output = {
            "total_splits": len(args.splits),
            "splits_processed": args.splits,
            "total_examples": len(all_results),
            "batch_size": args.batch_size,
            "max_new_tokens": args.max_new_tokens,
            "results": all_results
        }
        
        with open(args.eval_output, 'w') as f:
            json.dump(final_output, f, indent=2)
        
        # Clean up intermediate file if it exists
        intermediate_file = f"{args.eval_output}.intermediate"
        if os.path.exists(intermediate_file):
            os.remove(intermediate_file)
        
        print(f"\n{'='*60}")
        print("Evaluation Complete!")
        print(f"{'='*60}")
        print(f"Total splits processed: {len(args.splits)}")
        print(f"Total examples: {len(all_results)}")
        print(f"Batch size used: {args.batch_size}")
        print(f"Results saved to: {args.eval_output}")
        
    else:
        # Example inference for each split
        for split in args.splits:
            print(f"\n{'='*60}")
            print(f"Example inference for split: {split}")
            print(f"{'='*60}")
            
            # Load evaluation dataset for this split
            eval_dataset = load_dataset(
                args.dataset,
                split=split
            )
            
            # Get first example or batch
            num_examples = min(args.batch_size, len(eval_dataset))
            example_prompts = [eval_dataset[i]["prompt"][0]["content"] for i in range(num_examples)]
            
            print(f"Generating outputs for {num_examples} example(s)...")
            generated_outputs = generate_output_batch(
                example_prompts, sft_merged_model, tokenizer, stopping_criteria,
                max_new_tokens=args.max_new_tokens
            )
            
            # Display results for each example
            for i in range(num_examples):
                print(f"\n###### Example {i+1} from {split}: ######")
                print(example_prompts[i])

                # ground_truth_cot = eval_dataset[i].get("cot_steps", "")
                ground_truth_cot = ""
                ground_truth_code = eval_dataset[i].get("completion", [{}])[0].get("content", "")
                ground_truth = ground_truth_cot + ground_truth_code

                table = [
                    ["CoT-SFT Generated Output", "Ground Truth"],
                    [generated_outputs[i], ground_truth]
                ]
                print(tabulate(table, headers="firstrow", tablefmt="grid"))

                # save output to a text file
                with open(f"sft_model_example_output_{split}_ex{i}.txt", "w") as f:
                    f.write(f"Example {i+1} from {split}:\n")
                    f.write(example_prompts[i] + "\n\n")
                    f.write(tabulate(table, headers="firstrow", tablefmt="grid"))


if __name__ == "__main__":
    main()