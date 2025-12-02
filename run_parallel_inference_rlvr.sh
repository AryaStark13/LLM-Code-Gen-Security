#!/bin/bash
#SBATCH -N 1
#SBATCH -p GPU-shared
#SBATCH -t 24:00:00
#SBATCH --gpus=h100-80:1
#SBATCH -A cis240137p
#SBATCH --array=0-3                    # 4 jobs: index 0, 1, 2, 3
#SBATCH --output=inference/logs/rlvr_model_%a_output_%j.log
#SBATCH --error=inference/logs/rlvr_model_%a_error_%j.log

# Load modules
module load anaconda3/2024.10-1

# Activate environment
conda activate /ocean/projects/cis240137p/asheth1/conda/envs/.venv/

# Set cache to ocean
export HF_HOME="/ocean/projects/cis240137p/asheth1/.cache/huggingface"
export HF_DATASETS_CACHE="/ocean/projects/cis240137p/asheth1/.cache/huggingface/datasets"
export TRANSFORMERS_CACHE="/ocean/projects/cis240137p/asheth1/.cache/huggingface/transformers"
export TORCH_HOME="/ocean/projects/cis240137p/asheth1/.cache/torch"

# Set project directory
export PROJECT_DIR="/ocean/projects/cis240137p/asheth1/LLM-Code-Gen-Security"
cd $PROJECT_DIR

# mkdir -p inference/logs
# mkdir -p results/CoT_RLVR
# mkdir -p results/CWEval

# Define model configs - using local paths
MODELS_DIR="/ocean/projects/cis240137p/asheth1/hf_models"

BASE_MODELS=(
    "${MODELS_DIR}/google__codegemma-7b-it"
    "${MODELS_DIR}/meta-llama__CodeLlama-7b-Instruct-hf"
    "${MODELS_DIR}/deepseek-ai__deepseek-coder-7b-instruct-v1.5"
    "${MODELS_DIR}/Qwen__Qwen2.5-Coder-7B-Instruct"
)

SFT_ADAPTERS=(
    "${MODELS_DIR}/ShethArihant__PSC-2_codegemma-7b-it_sft_2-epochs"
    "${MODELS_DIR}/ShethArihant__PSC-2_CodeLlama-7b-Instruct-hf_sft_2-epochs"
    "${MODELS_DIR}/ShethArihant__PSC-2_deepseek-coder-7b-instruct-v1.5_sft_2-epochs"
    "${MODELS_DIR}/ShethArihant__PSC-2_Qwen2.5-Coder-7B-Instruct_sft_2-epochs"
)

RLVR_ADAPTERS=(
    "${MODELS_DIR}/lindafei001__gemma-7B-multilang"
    "${MODELS_DIR}/lindafei001__llama-7B-multilang"
    "${MODELS_DIR}/lindafei001__deepseek-7B-multilang"
    "${MODELS_DIR}/lindafei001__qwen-7B-multilang"
)

# Specify the checkpoint subfolder for each RLVR adapter
RLVR_ADAPTER_SUBFOLDERS=(
    "checkpoint-3000"
    "checkpoint-2700"
    "checkpoint-3800"
    "checkpoint-3100"
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
RLVR_ADAPTER=${RLVR_ADAPTERS[$SLURM_ARRAY_TASK_ID]}
RLVR_ADAPTER_SUBFOLDER=${RLVR_ADAPTER_SUBFOLDERS[$SLURM_ARRAY_TASK_ID]}
MODEL_NAME=${MODEL_NAMES[$SLURM_ARRAY_TASK_ID]}

echo "========================================"
echo "Job Array Task ID: $SLURM_ARRAY_TASK_ID"
echo "Base Model: $BASE_MODEL"
echo "SFT Adapter: $SFT_ADAPTER"
echo "RLVR Adapter: $RLVR_ADAPTER"
echo "RLVR Adapter Subfolder: $RLVR_ADAPTER_SUBFOLDER"
echo "Model Name: $MODEL_NAME"
echo "========================================"

nvidia-smi
date

# Set batch size for H100 80GB GPU
BATCH_SIZE=4

echo ""
echo "Starting inference on SeCodePLT dataset..."
echo "========================================"

# Create output directory for SeCodePLT
mkdir -p results/CoT_RLVR/${MODEL_NAME}/multilingual-RLVR

# Run inference on SeCodePLT dataset
python -m inference.inference_rlvr_batched \
    --dataset ShethArihant/SeCodePLT-updated-CoT-v4 \
    --splits test \
    --base_model $BASE_MODEL \
    --sft_lora_adapter $SFT_ADAPTER \
    --rlvr_lora_adapter $RLVR_ADAPTER \
    --rlvr_lora_adapter_subfolder $RLVR_ADAPTER_SUBFOLDER \
    --batch_size $BATCH_SIZE \
    --max_new_tokens 2048 \
    --run_eval \
    --eval_output results/CoT_RLVR/${MODEL_NAME}/multilingual-RLVR/SecCodePLT_CoT_SFT_Results.json

echo ""
echo "SeCodePLT inference complete!"
date

echo ""
echo "Starting inference on CWEval dataset..."
echo "========================================"

# Create output directory for CWEval
mkdir -p results/CWEval/${MODEL_NAME}/multilingual-RLVR

# Run inference on CWEval dataset with multiple splits
python -m inference.inference_rlvr_batched \
    --dataset ShethArihant/CWEval-v1 \
    --splits core_c core_cpp core_go core_js core_py lang_c \
    --base_model $BASE_MODEL \
    --sft_lora_adapter $SFT_ADAPTER \
    --rlvr_lora_adapter $RLVR_ADAPTER \
    --rlvr_lora_adapter_subfolder $RLVR_ADAPTER_SUBFOLDER \
    --batch_size $BATCH_SIZE \
    --max_new_tokens 2048 \
    --run_eval \
    --eval_output results/CWEval/${MODEL_NAME}/multilingual-RLVR/CWEval_Results.json

echo ""
echo "CWEval inference complete!"
date

echo ""
echo "========================================"
echo "All inference tasks complete for $MODEL_NAME!"
echo "========================================"
date