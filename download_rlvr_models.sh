#!/bin/bash
set -euo pipefail

# Put all model files under this directory
TARGET_DIR="/ocean/projects/cis240137p/asheth1/hf_models"

mkdir -p "$TARGET_DIR"

# Make sure HF cache also lives on /ocean (good practice on PSC)
export HF_HOME="/ocean/projects/cis240137p/asheth1/.cache/huggingface"
export HF_HUB_CACHE="$HF_HOME/hub"
export TRANSFORMERS_CACHE="$HF_HOME/transformers"

RLVR_ADAPTERS=(
    "lindafei001/gemma-7B-multilang"
    "lindafei001/llama-7B-multilang"
    "lindafei001/deepseek-7B-multilang"
    "lindafei001/qwen-7B-multilang"
)

download_model () {
    local repo="$1"
    # Replace slash with double underscore to make a folder name
    local subdir="${repo/\//__}"

    echo "Downloading $repo → $TARGET_DIR/$subdir"
    hf download "$repo" \
        --local-dir "$TARGET_DIR/$subdir"
}

for a in "${RLVR_ADAPTERS[@]}"; do
    download_model "$a"
done

echo "All RLVR adapter downloads complete."