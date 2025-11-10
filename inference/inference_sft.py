import unsloth # type: ignore[import]
from unsloth import FastLanguageModel # type: ignore[import]
import argparse
import torch
from transformers import AutoModelForCausalLM, AutoTokenizer
from peft import PeftModel
from transformers import StoppingCriteria, StoppingCriteriaList
from datasets import load_dataset
import json
from tabulate import tabulate
import os

device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

class CustomStopCriteria(StoppingCriteria):
    def __init__(self, stop_sequence_ids):
        self.stop_sequence_ids = stop_sequence_ids

    def __call__(self, input_ids: torch.LongTensor, scores: torch.FloatTensor, **kwargs) -> bool:
        # Check if the last part of the input_ids matches the stop sequence
        return input_ids[0, -len(self.stop_sequence_ids):].tolist() == self.stop_sequence_ids

def load_base_model_and_tokenizer(base_model_path):
    base_model = AutoModelForCausalLM.from_pretrained(
        base_model_path,
        torch_dtype=torch.float16,
        device_map="auto",
        trust_remote_code=True
    )

    tokenizer = AutoTokenizer.from_pretrained(base_model_path, trust_remote_code=True)
    return base_model, tokenizer

def load_sft_lora_adapter(base_model, sft_lora_adapter_path):
    sft_lora_model = PeftModel.from_pretrained(base_model, sft_lora_adapter_path)
    return sft_lora_model

def merge_sft_lora_adapter(sft_lora_model):
    merged_model = sft_lora_model.merge_and_unload()
    return merged_model

def generate_output(prompt: str, model, tokenizer, stopping_criteria, max_new_tokens=2048) -> str:
    inputs_tuned = tokenizer([prompt], return_tensors="pt").to(device)

    with torch.no_grad():
        outputs_tuned = model.generate(**inputs_tuned, max_new_tokens=max_new_tokens, stopping_criteria=stopping_criteria)

    outputs_tuned_str = tokenizer.decode(outputs_tuned[0][inputs_tuned["input_ids"].shape[-1]:], skip_special_tokens=True)
    return outputs_tuned_str

def run_full_evaluation(model, tokenizer, stopping_criteria, eval_dataset, output_file="evaluation_results.json"):
    """Run evaluation on the full dataset and save results as JSON"""
    # make sure output file directory exists
    output_dir = os.path.dirname(output_file)
    os.makedirs(output_dir, exist_ok=True)

    results = []
    
    for i in range(len(eval_dataset)):
        example = eval_dataset[i]
        
        # Extract prompt from the dataset
        prompt = example["prompt"][0]["content"]
        
        # Generate output
        print(f"Processing example {i+1}/{len(eval_dataset)}...")
        try:
            generated_output = generate_output(prompt, model, tokenizer, stopping_criteria)
        except Exception as e:
            print(f"Error generating output for example {i}: {e}")
            generated_output = ""
        
        # Create result entry
        result = {
            "index": i,
            "task_id": example.get("task_id", ""),
            "id": example.get("id", ""),
            "CWE_ID": example.get("CWE_ID", ""),
            "prompt": prompt,
            "ground_truth_cot": example.get("cot_steps", ""),
            "ground_truth_code": example.get("completion", "")[0]["content"],
            "ground_truth_full": example.get("cot_steps", "") + example.get("completion", "")[0]["content"],
            "y_negative": example.get("y_negative", ""),
            "output_without_tuning": "",
            "output_with_tuning": generated_output,
            "processed_at": i + 1,
            "has_non_tuned_output": False
        }
        
        results.append(result)
        
        # Save intermediate results every 10 examples
        if (i + 1) % 10 == 0:
            intermediate_output = {
                "total_examples": len(eval_dataset),
                "processed_examples": len(results),
                "results": results
            }
            with open(f"{output_file}.intermediate", 'w') as f:
                json.dump(intermediate_output, f, indent=2)
            print(f"Saved intermediate results ({len(results)} examples)")
    
    # Create final output structure
    output = {
        "total_examples": len(eval_dataset),
        "processed_examples": len(results),
        "results": results
    }
    
    # Save to JSON file
    with open(output_file, 'w') as f:
        json.dump(output, f, indent=2)
    
    print(f"Evaluation complete! Results saved to {output_file}")
    return output


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--dataset", type=str, help="Valid HF dataset path. Should be in the pre-processed format.")
    parser.add_argument("--split", type=str, default="python", help="Dataset split to use for evaluation")
    parser.add_argument("--base_model", type=str, default="deepseek-ai/deepseek-coder-7b-instruct-v1.5")
    parser.add_argument("--run_eval", action="store_true", help="Run evaluation on the full dataset")
    parser.add_argument("--eval_output", type=str, default="CWEval_Results.json", help="Output file for evaluation results")
    
    parser.add_argument("--sft_lora_adapter", type=str, default="ShethArihant/deepseek-coder-7b-instruct-v1.5_sft_2-epochs")

    args = parser.parse_args()

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

    # Load evaluation dataset
    eval_dataset = load_dataset(
        args.dataset,
        split=args.split
    )

    if args.run_eval:
        # Run full evaluation
        print("Starting full dataset evaluation...")
        run_full_evaluation(sft_merged_model, tokenizer, stopping_criteria, eval_dataset, args.eval_output)
    else:
        # Example inference
        example_prompt = eval_dataset[0]["prompt"][0]["content"]
        generated_output = generate_output(example_prompt, sft_merged_model, tokenizer, stopping_criteria)
        print("###### Example Prompt: ######")
        print(example_prompt)

        ground_truth_cot = eval_dataset[0]["cot_steps"]
        ground_truth_code = eval_dataset[0]["completion"][0]["content"]
        ground_truth = ground_truth_cot + ground_truth_code

        table = [
            ["CoT-SFT Generated Output", "Ground Truth"],
            [generated_output, ground_truth]
        ]
        print(tabulate(table, headers="firstrow", tablefmt="grid"))

        # save output to a text file
        with open("sft_model_example_output.txt", "w") as f:
            f.write("Example Prompt:\n")
            f.write(example_prompt + "\n\n")
            f.write(tabulate(table, headers="firstrow", tablefmt="grid"))


if __name__ == "__main__":
    main()
