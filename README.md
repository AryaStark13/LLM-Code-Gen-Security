# PSC Cluster Setup

1. Setup Conda and CUDA
```bash
module load anaconda3/2024.10-1
module load cuda/12.4.0
```

2. Activate Conda Environment:
```bash
conda activate /ocean/projects/cis240137p/asheth1/conda/envs/.venv/
```

3. (One-time Only) Install requirements:
```bash
pip install -r requirements/requirements_linux.txt
```

4. Request Compute Node:
```bash
# For H-100 (slower time get)
srun --partition=GPU-shared --gres=gpu:h100-80:1 --time=1:00:00 --pty bash

# For V-100 (faster to get)
srun --partition=GPU-shared --gres=gpu:v100-32:1 --time=1:00:00 --pty bash
```

5. Run Inference:
```bash
python -m inference.inference_sft \
--dataset ShethArihant/SeCodePLT-updated-CoT-v4 \
--split test \
--run_eval \
--eval_output "results/CoT_SFT/deepseek-coder-7b/CoT-SFT_3-datasets/SecCodePLT_CoT_SFT_Results.json"
```

OR
```bash
python -m inference.inference_sft \
--dataset ShethArihant/CWEval-v1 \
--splits core_c \
--eval_output "results/CWEval/deepseek-coder-7b/CoT-SFT_3-datasets/CWEval_Results.json"
```
python -m inference.inference_sft_batched \
--dataset ShethArihant/CWEval-v1 \
--splits core_c

# AWS EC2 Instance Setup Instructions

1. Clone the repository:
```bash
git clone https://github.com/AryaStark13/LLM-Code-Gen-Security.git
```
Open the directory in VS Code:

2. Setup Git User Config:
```bash
git config --global user.name "AryaStark13"
git config --global user.email "arihant.sheth0802@gmail.com"
```

3. Setup Python Environment:
```bash
python3 -m venv .venv
source .venv/bin/activate
pip install --upgrade pip
pip install -r requirements/requirements_linux.txt
```

4. Copy and populate .env file: Only `HF_TOKEN` and `WANDB_API_KEY` are mandatory for training.
```bash
cp .env.example .env
```

5. Run the SFT Training Script:
```bash
python -m training.sft_train \
--config training/configs/codegemma-config.yaml

python -m training.sft_train \
--config training/configs/codegemma-config.yaml 2>&1 | tee codegemma_training_$(date +%Y%m%d_%H%M%S).log
```

# Dataset Format
Each dataset should be pushed to HF Hub in the following format to be used for training/evaluation:

```yaml
task_id: "unique_task_identifier" # Global unique identifier for the task (Across datasets)
id: "unique_example_identifier" # Unique identifier for the example (Within dataset)
CWE_ID: 1234 # CWE ID associated with the example (-1 if not applicable)
prompt: [{"role": "user", "content": "What color is the sky?"}] # Prompt
completion: [{"role": "assistant", "content": "It is blue."}] # Expected completion
y_negative: "<code> vulnerable_code_snippet </code>" # Negative code snippet (Optional for all datasets)
cot_steps: "<think> step 1... step 2... </think>" # CoT steps (Optional for eval datasets)
```

## HF Dataset Schema
```python
{
    'task_id': Value('string'),
    'id': Value('string'),
    'CWE_ID': Value('int64'),
    'y_negative': Value('string'),
    'prompt': List({'content': Value('string'), 'role': Value('string')}),
    'cot_steps': Value('string'),
    'completion': List({'content': Value('string'), 'role': Value('string')})
}
```

You can check if your dataset conforms to this schema by running:
```python
from datasets import load_dataset, Value, List, DatasetDict, Features

dataset = load_dataset("your-hf-dataset-identifier")

ref_features = Features({
    'task_id': Value('string'),
    'id': Value('string'),
    'CWE_ID': Value('int64'),
    'y_negative': Value('string'),
    'prompt': List({'content': Value('string'), 'role': Value('string')}),
    'cot_steps': Value('string'),
    'completion': List({'content': Value('string'), 'role': Value('string')})
})

for split in dataset.keys():
    assert dataset[split].features == ref_features
```

# Run Inference

## SeCodePLT
```python
python -m inference.inference_sft \
--dataset ShethArihant/CWEval-v1 \
--split python
```

# SeCodePLT Unit Test Instructions

To execute the unit tests for a given `SecCodePLT_CoT_SFT_Results.json` file present in the `./results/CoT_SFT/<model-name>/` directory, follow these steps:

1. **Start the Server**:
```bash
python -m utils.SeCodePLT_updated.executor_docker.server
```
A server will start on `localhost:8666` by default. You can configure it to run on a different host or port by setting the `SERVER_PORT` environment variable present in the `utils/SeCodePLT_updated/.env` file. OR, by running the following command:
```bash
python -m executor_docker.server --port <desired-port>
```

2. **Run the Unit Tests**:
```bash
python utils/SeCodePLT_updated/python_eval_script.py --model_name <model-name>
```
Replace `<model-name>` with the actual name of the model whose results you want to test. `<model-name>` can only be one of the folders present in the `./results/CoT_SFT/` directory. This folder should contain the `SecCodePLT_CoT_SFT_Results.json` file.

# CWEval Unit Test Instructions

1. Convert the JSON results to CWEval format:
```bash
# Navigate to the root directory of the project
# the parent directory should point to the model results folder present in ./results/CWEval/*
# this should contain a file named CWEval_Results.json
python -m scripts.convert_cweval_json_to_eval \
--parent deepseek-coder-7b/CoT-SFT_RLVR-2
```

2. 
```bash
cd utils/CWEval
export EVAL_PATH=evals/<eval-path>
export EVAL_PATH=evals/eval_deepseek-coder-7b__<variant>
export EVAL_PATH=evals/eval_deepseek-coder-1b__<variant>
export EVAL_PATH=evals/eval_LLMs__<variant>

# Copy the task & test py files into the same folder as the generated outputs
python -m cweval.evaluate parse_generated \
--eval_path $EVAL_PATH

# Run Tests for python only
# eval_path is created in step 1 in the following format: evals/eval_<parent>__<model-name>
# run_tests will execute tests for only the language present
python -m cweval.evaluate run_tests \
--eval_path $EVAL_PATH

# Now execute merge results
python -m cweval.evaluate \
_merge_results \
--eval_path $EVAL_PATH

python -m cweval.evaluate \
report_pass_at_k \
--eval_path $EVAL_PATH \
--k 1 \
--lang core/py/

### ONLY for running the full pipeline (all languages) ###
# pipeline will execute tests for all languages
python cweval/evaluate.py pipeline \
--eval_path $EVAL_PATH \
--langs py
```

# General Code Dataset Language Distribution: Before Filtering
python      104,848
javascript  9,194
java        7,549
cpp         4,496
rust        2,226
php         2,005
c           1,374
ruby        573
go          507