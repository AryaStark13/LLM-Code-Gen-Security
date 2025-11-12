#!/bin/bash
#SBATCH -N 1
#SBATCH -p GPU-shared
#SBATCH -t 1:00:00
#SBATCH --gpus=v100-32:1
#SBATCH -A cis240137p
#SBATCH --output=test_training_%j.log
#SBATCH --error=test_training_%j.err

echo "Job started at $(date)"
echo "Working directory: $(pwd)"
echo "Job ID: $SLURM_JOB_ID"

# Load modules
echo "Loading modules..."
module load anaconda3/2024.10-1

# Activate environment
echo "Activating conda environment..."
conda activate /ocean/projects/cis240137p/asheth1/conda/envs/.venv/

# Show environment info
echo "Python: $(which python)"
echo "Python version: $(python --version)"

# Check GPU
echo "GPU info:"
nvidia-smi

# Set project directory
export PROJECT_DIR="/ocean/projects/cis240137p/asheth1/LLM-Code-Gen-Security"
cd $PROJECT_DIR || exit 1

echo "Changed to: $(pwd)"

# List files
echo "Files in training/:"
ls -l training/

echo "Config files:"
ls -l training/configs/

# Create output directory
echo "Creating results directory..."
mkdir -p results/training

# Test import
echo "Testing imports..."
python -c "import torch; import transformers; print(f'PyTorch: {torch.__version__}'); print(f'Transformers: {transformers.__version__}')"

echo "Environment check complete!"
