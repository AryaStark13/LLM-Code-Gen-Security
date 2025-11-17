# utils/CWEval/cweval/local_ai.py
import torch
from transformers import AutoModelForCausalLM, AutoTokenizer, StoppingCriteria, StoppingCriteriaList
from peft import PeftModel
from typing import Dict, List
import abc

device = torch.device("cuda" if torch.cuda.is_available() else "cpu")


class CustomStopCriteria(StoppingCriteria):
    def __init__(self, stop_sequence_ids):
        self.stop_sequence_ids = stop_sequence_ids

    def __call__(self, input_ids: torch.LongTensor, scores: torch.FloatTensor, **kwargs) -> bool:
        for i in range(input_ids.shape[0]):
            if input_ids[i, -len(self.stop_sequence_ids):].tolist() != self.stop_sequence_ids:
                return False
        return True


class LocalModelAPI:
    """API wrapper for local model with LoRA adapters"""
    
    def __init__(
        self,
        base_model_path: str,
        sft_lora_adapter_path: str = None,
        max_new_tokens: int = 2048,
        batch_size: int = 1,
        **kwargs,
    ):
        self.base_model_path = base_model_path
        self.sft_lora_adapter_path = sft_lora_adapter_path
        self.max_new_tokens = max_new_tokens
        self.batch_size = batch_size
        self.req_kwargs = kwargs
        
        print(f"Loading base model from {base_model_path}...")
        self.model, self.tokenizer = self._load_model_and_tokenizer()
        
        if sft_lora_adapter_path:
            print(f"Loading and merging SFT LoRA adapter from {sft_lora_adapter_path}...")
            self.model = self._load_and_merge_lora()
        
        # Setup stopping criteria for </code>
        stop_sequence_ids = self.tokenizer.encode("</code>", add_special_tokens=False)
        self.stopping_criteria = StoppingCriteriaList([CustomStopCriteria(stop_sequence_ids)])
        
        print("Model ready for inference")
        print(f"Total parameters: {sum(p.numel() for p in self.model.parameters()):,}")
    
    def _load_model_and_tokenizer(self):
        model = AutoModelForCausalLM.from_pretrained(
            self.base_model_path,
            torch_dtype=torch.float16,
            device_map="auto",
            trust_remote_code=True
        )
        
        tokenizer = AutoTokenizer.from_pretrained(
            self.base_model_path, 
            trust_remote_code=True
        )
        
        # Set pad token if not already set
        if tokenizer.pad_token is None:
            tokenizer.pad_token = tokenizer.eos_token
            tokenizer.pad_token_id = tokenizer.eos_token_id
        
        # Enable padding on the left for generation
        tokenizer.padding_side = "left"
        
        return model, tokenizer
    
    def _load_and_merge_lora(self):
        lora_model = PeftModel.from_pretrained(self.model, self.sft_lora_adapter_path)
        merged_model = lora_model.merge_and_unload()
        return merged_model
    
    def send_message(self, messages: List[Dict[str, str]], **kwargs) -> List[str]:
        """Generate responses for messages using local model"""
        all_kwargs = self.req_kwargs.copy()
        all_kwargs.update(kwargs)
        
        n_samples = all_kwargs.pop('n', 1)
        
        # Extract the prompt from messages
        prompt = messages[-1]['content']
        
        # Generate n_samples outputs
        all_outputs = []
        for _ in range(n_samples):
            output = self._generate_single(prompt)
            all_outputs.append(output)
        
        return all_outputs
    
    def _generate_single(self, prompt: str) -> str:
        """Generate a single output for a prompt"""
        inputs = self.tokenizer(
            prompt,
            return_tensors="pt",
            padding=True,
            truncation=True,
            max_length=4096
        ).to(device)
        
        with torch.no_grad():
            outputs = self.model.generate(
                **inputs,
                max_new_tokens=self.max_new_tokens,
                stopping_criteria=self.stopping_criteria,
                pad_token_id=self.tokenizer.pad_token_id,
                do_sample=True,  # Enable sampling for variety
                temperature=0.8,
            )
        
        # Decode only the generated part
        input_length = inputs["input_ids"].shape[1]
        generated_text = self.tokenizer.decode(
            outputs[0][input_length:], 
            skip_special_tokens=True
        )
        
        return generated_text