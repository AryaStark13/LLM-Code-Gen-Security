#!/bin/bash
#SBATCH -N 1
#SBATCH -p GPU-shared
#SBATCH -t 16:00:00
#SBATCH --gpus=v100-32:1
#SBATCH -A cis240137p
#SBATCH --array=0-2                    # 3 jobs: index 0, 1, 2
#SBATCH --output=results/training/model_%a_output_%j.log
#SBATCH --error=results/training/model_%a_error_%j.log

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

mkdir -p results/training

# Define model configs
CONFIGS=("qwen-config.yaml" "codellama-config.yaml" "codegemma-config.yaml")
MODEL_NAMES=("Qwen2.5-Coder" "CodeLlama" "CodeGemma")

# Get config for this array task
CONFIG=${CONFIGS[$SLURM_ARRAY_TASK_ID]}
MODEL=${MODEL_NAMES[$SLURM_ARRAY_TASK_ID]}

echo "========================================"
echo "Job Array Task ID: $SLURM_ARRAY_TASK_ID"
echo "Training: $MODEL"
echo "Config: $CONFIG"
echo "========================================"

nvidia-smi
date

# Train the model
python training/sft_train.py \
  --config training/configs/$CONFIG

echo ""
echo "$MODEL training complete!"
date
