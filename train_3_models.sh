#!/bin/bash
#SBATCH -N 1
#SBATCH -p GPU-shared
#SBATCH -t 48:00:00
#SBATCH --gpus=v100-32:2
#SBATCH -A cis240137p
#SBATCH --output=results/training/training_output_%j.log
#SBATCH --error=results/training/training_error_%j.log

# Load modules
module load anaconda3/2024.10-1

# Activate environment
conda activate /ocean/projects/cis240137p/asheth1/conda/envs/.venv/

# Set project directory
export PROJECT_DIR="/ocean/projects/cis240137p/asheth1/LLM-Code-Gen-Security"
cd $PROJECT_DIR

# Create results directory if it doesn't exist
mkdir -p results/training

# Show GPU info
nvidia-smi
echo "Starting training for 3 models..."
date

# Start background GPU monitor
nvidia-smi dmon -s u -d 30 > results/training/gpu_monitor_${SLURM_JOB_ID}.log 2>&1 &
MONITOR_PID=$!

echo "========================================"
echo "Training Model 1/3: Qwen2.5-Coder-7B-Instruct"
echo "========================================"
python training/sft_train.py \
  --config training/configs/qwen-config.yaml

echo ""
echo "Model 1 complete. Clearing GPU cache..."
python -c "import torch; torch.cuda.empty_cache()"
sleep 10

echo "========================================"
echo "Training Model 2/3: CodeLlama-7b-Instruct-hf"
echo "========================================"
python training/sft_train.py \
  --config training/configs/codellama-config.yaml

echo ""
echo "Model 2 complete. Clearing GPU cache..."
python -c "import torch; torch.cuda.empty_cache()"
sleep 10

echo "========================================"
echo "Training Model 3/3: CodeGemma-7b-it"
echo "========================================"
python training/sft_train.py \
  --config training/configs/codegemma-config.yaml

echo ""
echo "All models complete!"

# Kill GPU monitor
kill $MONITOR_PID

echo "========================================"
echo "Training Summary"
echo "========================================"
echo "Completed training for:"
echo "1. Qwen/Qwen2.5-Coder-7B-Instruct"
echo "2. meta-llama/CodeLlama-7b-Instruct-hf"
echo "3. google/codegemma-7b-it"
echo ""
echo "Check WandB for training metrics"
echo "Check HuggingFace Hub for saved models"
date