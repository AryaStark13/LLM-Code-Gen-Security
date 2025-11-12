#!/bin/bash
#SBATCH -N 1
#SBATCH -p GPU-shared
#SBATCH -t 3:00:00
#SBATCH --gpus=v100-32:1
#SBATCH --output=results/CWEval/deepseek-coder-7b/CoT-SFT_3-datasets-2/inference_output_%j.log      # Standard output file (%j will be replaced with job ID)
#SBATCH --error=results/CWEval/deepseek-coder-7b/CoT-SFT_3-datasets-2/inference_error_%j.log        # Standard error file (%j will be replaced with job ID)

# load conda
module load anaconda3/2024.10-1

# activate environment
conda activate /ocean/projects/cis240137p/asheth1/conda/envs/.venv/
nvidia-smi

export PROJECT_DIR="/ocean/projects/cis240137p/asheth1/LLM-Code-Gen-Security"
cd $PROJECT_DIR

# execute your test
python -m inference.inference_sft_batched \
  --dataset ShethArihant/CWEval-v1 \
  --splits core_c core_cpp core_go core_js core_py lang_c \
  --batch_size 4 \
  --run_eval \
  --eval_output "results/CWEval/deepseek-coder-7b/CoT-SFT_3-datasets-2/CWEval_Results.json"