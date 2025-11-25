#!/bin/bash
#SBATCH -N 1
#SBATCH -p GPU-shared
#SBATCH -t 24:00:00
#SBATCH --gpus=h100-80:1
#SBATCH -A cis240137p
#SBATCH --array=0-3                    # 4 jobs: index 0, 1, 2, 3
#SBATCH --output=inference/logs/model_%a_output_%j.log
#SBATCH --error=inference/logs/model_%a_error_%j.log

# Load modules
module load anaconda3/2024.10-1

# Activate environment
conda activate /ocean/projects/cis240137p/asheth1/conda/envs/.venv/

# Set cache to ocean (PSC-friendly)
export HF_HOME="/ocean/projects/cis240137p/asheth1/.cache/huggingface"
export HF_DATASETS_CACHE="/ocean/projects/cis240137p/asheth1/.cache/huggingface/datasets"
export TRANSFORMERS_CACHE="/ocean/projects/cis240137p/asheth1/.cache/huggingface/transformers"
export TORCH_HOME="/ocean/projects/cis240137p/asheth1/.cache/torch"

# Set project directory
export PROJECT_DIR="/ocean/projects/cis240137p/asheth1/LLM-Code-Gen-Security"
cd "$PROJECT_DIR"

# Base directory where you downloaded models with `hf download`
HF_MODELS_DIR="/ocean/projects/cis240137p/asheth1/hf_models"

# Define model configs using LOCAL PATHS now
BASE_MODELS=(
    "${HF_MODELS_DIR}/google__codegemma-7b-it"
    "${HF_MODELS_DIR}/meta-llama__CodeLlama-7b-Instruct-hf"
    "${HF_MODELS_DIR}/deepseek-ai__deepseek-coder-7b-instruct-v1.5"
    "${HF_MODELS_DIR}/Qwen__Qwen2.5-Coder-7B-Instruct"
)

SFT_ADAPTERS=(
    "${HF_MODELS_DIR}/ShethArihant__PSC-2_codegemma-7b-it_sft_2-epochs"
    "${HF_MODELS_DIR}/ShethArihant__PSC-2_CodeLlama-7b-Instruct-hf_sft_2-epochs"
    "${HF_MODELS_DIR}/ShethArihant__PSC-2_deepseek-coder-7b-instruct-v1.5_sft_2-epochs"
    "${HF_MODELS_DIR}/ShethArihant__PSC-2_Qwen2.5-Coder-7B-Instruct_sft_2-epochs"
)

MODEL_NAMES=(
    "codegemma-7b-it"
    "CodeLlama-7b-Instruct-hf"
    "deepseek-coder-7b-instruct-v1.5"
    "Qwen2.5-Coder-7B-Instruct"
)

# Get config for this array task
BASE_MODEL=${BASE_MODELS[$SLURM_ARRAY_TASK_ID]}
SFT_ADAPTER=${SFT_ADAPTERS[$SLURM_ARRAY_TASK_ID]}
MODEL_NAME=${MODEL_NAMES[$SLURM_ARRAY_TASK_ID]}

echo "========================================"
echo "Job Array Task ID: $SLURM_ARRAY_TASK_ID"
echo "Base Model: $BASE_MODEL"
echo "SFT Adapter: $SFT_ADAPTER"
echo "Model Name: $MODEL_NAME"
echo "========================================"

nvidia-smi
date

# Set batch size for H100 80GB GPU
BATCH_SIZE=4

echo ""
echo "Starting inference on CWEval dataset..."
echo "========================================"

# Create output directory for CWEval
mkdir -p "results/CWEval/${MODEL_NAME}/CoT-SFT_3-datasets"

python -m inference.inference_sft_batched \
    --dataset ShethArihant/CWEval-v1 \
    --splits core_c core_cpp core_go core_js core_py lang_c \
    --base_model "$BASE_MODEL" \
    --sft_lora_adapter "$SFT_ADAPTER" \
    --batch_size "$BATCH_SIZE" \
    --max_new_tokens 2048 \
    --run_eval \
    --eval_output "results/CWEval/${MODEL_NAME}/CoT-SFT_3-datasets/CWEval_code_analysis.json"

echo ""
echo "CWEval inference complete!"
date

echo ""
echo "========================================"
echo "All inference tasks complete for $MODEL_NAME!"
echo "========================================"
date
