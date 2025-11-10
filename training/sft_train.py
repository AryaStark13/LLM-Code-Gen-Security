#!/usr/bin/env python3
"""
Fine-tuning script with LoRA support and custom triplet loss.
Uses YAML configuration file for all parameters.
"""

import os
import warnings
from dotenv import load_dotenv
from dataclasses import dataclass
from typing import Optional, List, Dict
import argparse

import torch
import torch.nn as nn
import yaml
from datasets import load_dataset, concatenate_datasets
from huggingface_hub import login
from peft import LoraConfig, get_peft_model, TaskType, prepare_model_for_kbit_training
from transformers import AutoModelForCausalLM, AutoTokenizer, PreTrainedTokenizerBase
from transformers.data.data_collator import DataCollatorMixin
from trl import SFTTrainer, SFTConfig

import wandb

load_dotenv()
warnings.filterwarnings("ignore")


def load_config(config_path: str) -> dict:
    """Load configuration from YAML file."""
    with open(config_path, 'r') as f:
        return yaml.safe_load(f)


def get_token_representation(hidden_states, attention_mask, method="mean_pooled"):
    """
    Extract token representation from hidden states.
    
    Args:
        hidden_states: Model hidden states
        attention_mask: Attention mask
        method: "mean_pooled" or "last_token"
    
    Returns:
        Tensor of shape [batch_size, hidden_size]
    """
    last_hidden = hidden_states[-1]  # Last layer: [batch_size, seq_len, hidden_size]
    
    if method == "mean_pooled":
        if attention_mask is not None:
            mask_expanded = attention_mask.unsqueeze(-1).expand(last_hidden.size()).float()
            sum_hidden = torch.sum(last_hidden * mask_expanded, dim=1)
            sum_mask = torch.clamp(mask_expanded.sum(1), min=1e-9)
            return sum_hidden / sum_mask
        else:
            return torch.mean(last_hidden, dim=1)
    
    elif method == "last_token":
        if attention_mask is not None:
            sequence_lengths = attention_mask.sum(dim=1) - 1  # [batch_size]
            batch_size = last_hidden.shape[0]
            return last_hidden[torch.arange(batch_size, device=last_hidden.device), sequence_lengths]
        else:
            return last_hidden[:, -1, :]
    
    else:
        raise ValueError(f"Unknown representation method: {method}")


@dataclass
class TripletDataCollator(DataCollatorMixin):
    """Data collator for triplet loss training."""
    
    tokenizer: PreTrainedTokenizerBase
    max_length: Optional[int] = None
    pad_to_multiple_of: Optional[int] = None
    return_tensors: str = "pt"

    def torch_call(self, examples):
        # batch = {}
        
        # # Extract components
        # prompts = [
        #     ex['prompt'][0]['content'] if isinstance(ex['prompt'], list) else ex['prompt'] 
        #     for ex in examples
        # ]
        # # check that all prompts are not None
        # for i, p in enumerate(prompts):
        #     if p is None:
        #         print(f"Example {i} has None prompt")
        #         print(examples[i])
        #         raise ValueError(f"Example {i} has None prompt")

        # cot_steps = [ex['cot_steps'] for ex in examples]
        # codes = [
        #     ex['completion'][0]['content'] if isinstance(ex['completion'], list) else ex['completion'] 
        #     for ex in examples
        # ]
        # negatives = [ex['y_negative'] for ex in examples]

        batch = {}
        
        # Extract components with detailed error checking
        prompts = []
        cot_steps = []
        codes = []
        negatives = []
        
        for i, ex in enumerate(examples):
            # Extract prompt
            if isinstance(ex['prompt'], list):
                p = ex['prompt'][0]['content'] if ex['prompt'] else None
            else:
                p = ex['prompt']
            
            if p is None:
                print(f"ERROR: Example {i} has None prompt")
                print(f"Full example: {ex}")
                raise ValueError(f"Example {i} has None prompt")
            prompts.append(p)
            
            # Extract cot_steps
            cot = ex.get('cot_steps', '')
            if cot is None:
                print(f"WARNING: Example {i} has None cot_steps, using empty string")
                print(f"Dataset: {ex.get('task_id', 'unknown')}")
                cot = ''
            cot_steps.append(cot)
            
            # Extract completion
            if isinstance(ex['completion'], list):
                code = ex['completion'][0]['content'] if ex['completion'] else None
            else:
                code = ex['completion']
            
            if code is None:
                print(f"ERROR: Example {i} has None completion")
                print(f"Full example: {ex}")
                raise ValueError(f"Example {i} has None completion")
            codes.append(code)
            
            # Extract negative
            neg = ex.get('y_negative', '')
            if neg is None:
                print(f"WARNING: Example {i} has None y_negative, using empty string")
                neg = ''
            negatives.append(neg)

        # Tokenize full sequence (prompt + cot + code) for CE loss
        prompt_cot_code_texts = [
            p + cot + code + self.tokenizer.eos_token
            for p, cot, code in zip(prompts, cot_steps, codes)
        ]
        prompt_cot_code_encodings = self.tokenizer(
            prompt_cot_code_texts,
            padding=True,
            truncation=True,
            max_length=self.max_length,
            return_tensors=self.return_tensors,
            pad_to_multiple_of=self.pad_to_multiple_of
        )

        # Tokenize prompts only (for triplet loss anchor)
        prompts_with_eos = [p + self.tokenizer.eos_token for p in prompts]
        prompt_encodings = self.tokenizer(
            prompts_with_eos,
            padding=True,
            truncation=True,
            max_length=self.max_length,
            return_tensors=self.return_tensors,
            pad_to_multiple_of=self.pad_to_multiple_of
        )

        # Tokenize cot + code (for triplet loss positive)
        cot_code_texts = [
            cot + code + self.tokenizer.eos_token
            for cot, code in zip(cot_steps, codes)
        ]
        positive_encodings = self.tokenizer(
            cot_code_texts,
            padding=True,
            truncation=True,
            max_length=self.max_length,
            return_tensors=self.return_tensors,
            pad_to_multiple_of=self.pad_to_multiple_of
        )

        # Tokenize negatives (for triplet loss negative)
        negatives_with_eos = [neg + self.tokenizer.eos_token for neg in negatives]
        negative_encodings = self.tokenizer(
            negatives_with_eos,
            padding=True,
            truncation=True,
            max_length=self.max_length,
            return_tensors=self.return_tensors,
            pad_to_multiple_of=self.pad_to_multiple_of
        )

        # Calculate prompt lengths for CE loss masking
        prompt_lengths = []
        for prompt in prompts_with_eos:
            prompt_tokens = self.tokenizer(prompt, return_tensors="pt")["input_ids"]
            prompt_lengths.append(prompt_tokens.size(1))

        # Build batch
        batch.update(prompt_cot_code_encodings)
        batch['labels'] = prompt_cot_code_encodings['input_ids'].clone()
        batch['prompt_input_ids'] = prompt_encodings['input_ids']
        batch['prompt_attention_mask'] = prompt_encodings['attention_mask']
        batch['positive_input_ids'] = positive_encodings['input_ids']
        batch['positive_attention_mask'] = positive_encodings['attention_mask']
        batch['negative_input_ids'] = negative_encodings['input_ids']
        batch['negative_attention_mask'] = negative_encodings['attention_mask']
        batch['prompt_lengths'] = torch.tensor(prompt_lengths, dtype=torch.long)

        return batch


class CustomSFTTrainer(SFTTrainer):
    """Custom SFTTrainer with combined cross-entropy and triplet loss."""

    def __init__(
        self,
        ce_loss_weight: float = 0.8,
        triplet_loss_weight: float = 0.2,
        triplet_margin: float = 1.0,
        token_representation: str = "mean_pooled",
        **kwargs
    ):
        self.ce_loss_weight = ce_loss_weight
        self.triplet_loss_weight = triplet_loss_weight
        self.triplet_margin = triplet_margin
        self.token_representation = token_representation
        
        self.ce_loss_fn = nn.CrossEntropyLoss(ignore_index=-100)
        
        # Only initialize triplet loss if weight > 0
        if self.triplet_loss_weight > 0:
            self.triplet_loss_fn = nn.TripletMarginLoss(margin=triplet_margin)
        
        super().__init__(**kwargs)

    def compute_loss(self, model, inputs, return_outputs=False, num_items_in_batch=None):
        """Compute combined CE + triplet loss with memory optimization."""
        is_eval = not model.training
        
        # Extract inputs
        main_input_ids = inputs['input_ids']
        main_attention_mask = inputs['attention_mask']
        prompt_lengths = inputs['prompt_lengths']

        # Forward pass for main sequence
        with torch.cuda.amp.autocast(dtype=torch.bfloat16):
            outputs_main = model(
                input_ids=main_input_ids,
                attention_mask=main_attention_mask,
                output_hidden_states=(self.triplet_loss_weight > 0 and not is_eval),
                use_cache=False
            )

        # Compute CE loss on completion tokens only
        batch_size = main_input_ids.size(0)
        ce_losses = []

        for i in range(batch_size):
            prompt_len = prompt_lengths[i].item()
            logits_completion = outputs_main.logits[i, prompt_len:-1, :]
            targets_completion = main_input_ids[i, prompt_len+1:]

            min_len = min(logits_completion.size(0), targets_completion.size(0))
            if min_len > 0:
                ce_loss_i = self.ce_loss_fn(logits_completion[:min_len], targets_completion[:min_len])
                ce_losses.append(ce_loss_i)

        ce_loss = torch.stack(ce_losses).mean() if ce_losses else torch.tensor(
            0.0, device=main_input_ids.device, requires_grad=True
        )

        # Store or delete outputs
        if return_outputs:
            outputs_to_return = outputs_main
        else:
            del outputs_main
            torch.cuda.empty_cache()

        # Skip triplet loss during eval or if weight is 0
        if is_eval or self.triplet_loss_weight == 0:
            total_loss = ce_loss
            
            if is_eval:
                self.log({
                    'eval/ce_loss': ce_loss.item(),
                    'eval/total_loss': total_loss.item(),
                })
            
            return (total_loss, outputs_to_return) if return_outputs else total_loss

        # Compute triplet loss (training only, weight > 0)
        with torch.cuda.amp.autocast(dtype=torch.bfloat16):
            # Anchor (prompt)
            outputs_prompt = model(
                input_ids=inputs['prompt_input_ids'],
                attention_mask=inputs['prompt_attention_mask'],
                output_hidden_states=True,
                use_cache=False
            )
            prompt_repr = get_token_representation(
                outputs_prompt.hidden_states,
                inputs['prompt_attention_mask'],
                self.token_representation
            )
            del outputs_prompt
            torch.cuda.empty_cache()

            # Positive (cot + code)
            outputs_positive = model(
                input_ids=inputs['positive_input_ids'],
                attention_mask=inputs['positive_attention_mask'],
                output_hidden_states=True,
                use_cache=False
            )
            positive_repr = get_token_representation(
                outputs_positive.hidden_states,
                inputs['positive_attention_mask'],
                self.token_representation
            )
            del outputs_positive
            torch.cuda.empty_cache()

            # Negative
            outputs_negative = model(
                input_ids=inputs['negative_input_ids'],
                attention_mask=inputs['negative_attention_mask'],
                output_hidden_states=True,
                use_cache=False
            )
            negative_repr = get_token_representation(
                outputs_negative.hidden_states,
                inputs['negative_attention_mask'],
                self.token_representation
            )
            del outputs_negative
            torch.cuda.empty_cache()

        triplet_loss = self.triplet_loss_fn(prompt_repr, positive_repr, negative_repr)
        total_loss = (self.ce_loss_weight * ce_loss) + (self.triplet_loss_weight * triplet_loss)

        self.log({
            'train/ce_loss': ce_loss.item(),
            'train/triplet_loss': triplet_loss.item(),
            'train/total_loss': total_loss.item(),
        })

        return (total_loss, outputs_to_return) if return_outputs else total_loss

    def evaluation_loop(self, *args, **kwargs):
        """Override evaluation loop for memory management."""
        torch.cuda.empty_cache()
        result = super().evaluation_loop(*args, **kwargs)
        torch.cuda.empty_cache()
        return result


def load_datasets(dataset_configs: List[Dict]) -> tuple:
    """
    Load and concatenate multiple datasets.
    
    Args:
        dataset_configs: List of dicts with 'name', 'train_splits', 'eval_splits'
        train_splits is a string or list of strings for training splits
        eval_splits is a string or list of strings for evaluation splits (optional)
        if eval_splits is not provided, that dataset is not used for evaluation.
    
    Returns:
        (train_dataset, eval_dataset)
    """
    train_datasets = []
    eval_datasets = []
    
    for config in dataset_configs:
        name = config['name']
        train_splits = config['train_splits']
        eval_splits = config.get('eval_splits', None)
        
        print(f"Loading dataset: {name}")
        print(f"- Train splits: {train_splits}")
        if eval_splits:
            print(f"- Eval splits: {eval_splits}")
        else:
            print("- No eval splits specified")

        # Load and concatenate train splits
        if isinstance(train_splits, str):
            train_splits = [train_splits]

        for train_split in train_splits:
            train_ds = load_dataset(name, split=train_split)
            train_datasets.append(train_ds)

        # Load and concatenate eval splits
        if eval_splits:
            if isinstance(eval_splits, str):
                eval_splits = [eval_splits]

            for eval_split in eval_splits:
                eval_ds = load_dataset(name, split=eval_split)
                eval_datasets.append(eval_ds)

    # Concatenate all datasets
    train_dataset = concatenate_datasets(train_datasets) if len(train_datasets) > 1 else train_datasets[0]
    eval_dataset = concatenate_datasets(eval_datasets) if len(eval_datasets) > 1 else eval_datasets[0]

    # Shuffle datasets
    train_dataset = train_dataset.shuffle(seed=42)
        
    #     train_ds = load_dataset(name, split=train_split)
    #     eval_ds = load_dataset(name, split=eval_split)
        
    #     train_datasets.append(train_ds)
    #     eval_datasets.append(eval_ds)
    
    # # Concatenate all datasets
    # train_dataset = concatenate_datasets(train_datasets) if len(train_datasets) > 1 else train_datasets[0]
    # eval_dataset = concatenate_datasets(eval_datasets) if len(eval_datasets) > 1 else eval_datasets[0]
    
    print(f"\nTotal: {len(train_dataset)} train, {len(eval_dataset)} eval examples")
    
    return train_dataset, eval_dataset


def main():
    # Argument parsing
    parser = argparse.ArgumentParser(description="Fine-tune a language model with LoRA and triplet loss.")
    parser.add_argument("--config", type=str, default="training/configs/sft-train-config.yaml", help="Path to the YAML configuration file.")
    args = parser.parse_args()

    # Load configuration
    config = load_config(args.config)
    
    # Login to HuggingFace and WandB
    hf_token = os.environ.get('HF_TOKEN')
    wandb_key = os.environ.get('WANDB_API_KEY')

    # Check if tokens are provided
    if not hf_token:
        print("Warning: HF_TOKEN not found in environment variables.")
        raise ValueError("HuggingFace token (HF_TOKEN) is required.")
    if not wandb_key:
        print("Warning: WANDB_API_KEY not found in environment variables.")
        raise ValueError("WandB API key (WANDB_API_KEY) is required.")

    
    if hf_token:
        login(token=hf_token)
    
    if wandb_key:
        wandb.login(key=wandb_key)
    
    # Setup device
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"Using device: {device}")
    assert device == torch.device("cuda"), "CUDA is required"
    
    # Load datasets
    train_dataset, eval_dataset = load_datasets(config['datasets'])
    
    # Load model and tokenizer
    model_name = config['model']['name']
    print(f"\nLoading model: {model_name}")
    
    tokenizer = AutoTokenizer.from_pretrained(model_name)
    base_model = AutoModelForCausalLM.from_pretrained(
        model_name,
        torch_dtype=torch.bfloat16,
        device_map="auto"
    )
    
    # Apply LoRA if enabled
    lora_config = config['lora']
    if lora_config['enabled']:
        print("\n" + "="*50)
        print("Applying LoRA configuration...")
        print("="*50)
        
        # base_model = prepare_model_for_kbit_training(base_model)# base_model = prepare_model_for_kbit_training(base_model)
        
        peft_config = LoraConfig(
            r=lora_config['r'],
            lora_alpha=lora_config['alpha'],
            target_modules=lora_config['target_modules'],
            lora_dropout=lora_config['dropout'],
            bias="none",
            task_type=TaskType.CAUSAL_LM,
        )
        
        model = get_peft_model(base_model, peft_config)
        model.print_trainable_parameters()
        print("="*50 + "\n")
    else:
        model = base_model
        print("Using full fine-tuning (no LoRA)")
    
    # Setup data collator
    data_collator = TripletDataCollator(
        tokenizer=tokenizer,
        max_length=config['training']['max_output_length']
    )
    
    # Create WandB run name
    model_short_name = model_name.split("/")[-1]
    epochs = config['training']['epochs']
    wandb_run_name = f"{model_short_name}_sft_{epochs}-epochs"
    
    # Initialize WandB
    dataset_names = [d['name'] for d in config['datasets']]
    wandb.init(
        project=config['wandb']['project'],
        entity=config['wandb'].get('entity'),
        name=wandb_run_name,
        config={
            "model": model_name,
            "datasets": dataset_names,
            "use_lora": lora_config['enabled'],
            "lora_r": lora_config['r'] if lora_config['enabled'] else None,
            "lora_alpha": lora_config['alpha'] if lora_config['enabled'] else None,
            "lora_dropout": lora_config['dropout'] if lora_config['enabled'] else None,
            "ce_loss_weight": config['training']['ce_loss_weight'],
            "triplet_loss_weight": config['training']['triplet_loss_weight'],
            "token_representation": config['training']['token_representation'],
        }
    )
    
    # Setup SFT configuration
    hub_model_id = f"{config['hub']['profile_name']}/{wandb_run_name}"
    
    sft_config_dict = config['sft_config'].copy()
    sft_config_dict.update({
        'output_dir': wandb_run_name,
        'max_length': config['training']['max_output_length'],
        'remove_unused_columns': False,
        'per_device_train_batch_size': config['training']['batch_size'],
        'num_train_epochs': config['training']['epochs'],
        'learning_rate': config['training']['learning_rate'],
        'eval_steps': config['training']['eval_steps'],
        'save_steps': config['training']['save_steps'],
        'eval_strategy': 'steps',
        'save_strategy': 'steps',
        'report_to': 'wandb',
        'run_name': wandb_run_name,
        'push_to_hub': config['hub']['push_to_hub'],
        'hub_model_id': hub_model_id,
        'save_total_limit': config['hub']['save_total_limit'],
    })
    
    sft_config = SFTConfig(**sft_config_dict)
    
    # Create trainer
    trainer = CustomSFTTrainer(
        model=model,
        args=sft_config,
        train_dataset=train_dataset,
        eval_dataset=eval_dataset,
        data_collator=data_collator,
        processing_class=tokenizer,
        ce_loss_weight=config['training']['ce_loss_weight'],
        triplet_loss_weight=config['training']['triplet_loss_weight'],
        triplet_margin=config['training']['triplet_loss_margin'],
        token_representation=config['training']['token_representation'],
    )
    
    # Print training configuration
    print(f"\nTraining Configuration:")
    print(f"- Model: {model_name}")
    print(f"- Datasets: {', '.join(dataset_names)}")
    print(f"- Using LoRA: {lora_config['enabled']}")
    if lora_config['enabled']:
        print(f"- LoRA rank (r): {lora_config['r']}")
        print(f"- LoRA alpha: {lora_config['alpha']}")
    print(f"- Epochs: {epochs}")
    print(f"- CE loss weight: {config['training']['ce_loss_weight']}")
    print(f"- Triplet loss weight: {config['training']['triplet_loss_weight']}")
    print(f"- Token representation: {config['training']['token_representation']}")
    print(f"- Hub Model ID: {hub_model_id}")
    
    # Train
    trainer.train()
    
    # Save final model
    if lora_config['enabled']:
        print("\nSaving final LoRA adapter...")
        model.save_pretrained(wandb_run_name)
        tokenizer.save_pretrained(wandb_run_name)
        print(f"LoRA adapter saved to {wandb_run_name}")
    
    print("\nTraining complete!")


if __name__ == "__main__":
    main()