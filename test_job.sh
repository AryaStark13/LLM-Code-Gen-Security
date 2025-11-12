#!/bin/bash
#SBATCH -N 1
#SBATCH -p GPU-shared
#SBATCH -t 3:00:00
#SBATCH --gpus=v100-32:1
#SBATCH --output=batch_inference_output_%j.log      # Standard output file (%j will be replaced with job ID)
#SBATCH --error=batch_inference_error_%j.log        # Standard error file (%j will be replaced with job ID)

# load conda
module load anaconda3/2024.10-1

# activate environment
conda activate /ocean/projects/cis240137p/asheth1/conda/envs/.venv/
nvidia-smi

export PROJECT_DIR="/ocean/projects/cis240137p/asheth1/LLM-Code-Gen-Security"
cd $PROJECT_DIR

# test commands
pwd

date
# run the Unix 'date' command
echo "Hello world, from Bridges-2!"
