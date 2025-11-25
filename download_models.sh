#!/bin/bash
set -euo pipefail

# Put all model files under this directory on /ocean
TARGET_DIR="/ocean/projects/cis240137p/asheth1/hf_models"

mkdir -p "$TARGET_DIR"

# Make sure HF cache also lives on /ocean (good practice on PSC)
export HF_HOME="/ocean/projects/cis240137p/asheth1/.cache/huggingface"
export HF_HUB_CACHE="$HF_HOME/hub"
export TRANSFORMERS_CACHE="$HF_HOME/transformers"

BASE_MODELS=(
    "google/codegemma-7b-it"
    "meta-llama/CodeLlama-7b-Instruct-hf"
    "deepseek-ai/deepseek-coder-7b-instruct-v1.5"
    "Qwen/Qwen2.5-Coder-7B-Instruct"
)

SFT_ADAPTERS=(
    "ShethArihant/PSC-2_codegemma-7b-it_sft_2-epochs"
    "ShethArihant/PSC-2_CodeLlama-7b-Instruct-hf_sft_2-epochs"
    "ShethArihant/PSC-2_deepseek-coder-7b-instruct-v1.5_sft_2-epochs"
    "ShethArihant/PSC-2_Qwen2.5-Coder-7B-Instruct_sft_2-epochs"
)

download_model () {
    local repo="$1"
    # Replace slash with double underscore to make a folder name
    local subdir="${repo/\//__}"

    echo "Downloading $repo → $TARGET_DIR/$subdir"
    hf download "$repo" \
        --local-dir "$TARGET_DIR/$subdir"
}

for m in "${BASE_MODELS[@]}"; do
    download_model "$m"
done

for a in "${SFT_ADAPTERS[@]}"; do
    download_model "$a"
done

echo "All downloads complete."
