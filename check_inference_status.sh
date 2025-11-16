#!/bin/bash

# Script to check the status of parallel inference jobs

echo "========================================"
echo "Parallel Inference Status Monitor"
echo "========================================"
echo ""

# Check SLURM queue
echo "Current job status:"
echo "-------------------"
squeue -u $USER -o "%.18i %.9P %.30j %.8T %.10M %.6D %R"
echo ""

# Check for result files
echo "Result files status:"
echo "--------------------"

MODELS=("codegemma-7b-it" "CodeLlama-7b-Instruct-hf" "deepseek-coder-7b-instruct-v1.5" "Qwen2.5-Coder-7B-Instruct")

for i in "${!MODELS[@]}"; do
    MODEL=${MODELS[$i]}
    echo ""
    echo "[$i] $MODEL:"
    
    # Check SeCodePLT results
    SECODEPLT_FILE="results/CoT_SFT/${MODEL}/SFT_3-datasets/SecCodePLT_CoT_SFT_Results.json"
    if [ -f "$SECODEPLT_FILE" ]; then
        SIZE=$(du -h "$SECODEPLT_FILE" | cut -f1)
        EXAMPLES=$(grep -o '"index":' "$SECODEPLT_FILE" | wc -l)
        echo "  ✓ SeCodePLT: $SIZE ($EXAMPLES examples)"
    else
        echo "  ✗ SeCodePLT: Not found"
        # Check for intermediate file
        if [ -f "${SECODEPLT_FILE}.intermediate" ]; then
            SIZE=$(du -h "${SECODEPLT_FILE}.intermediate" | cut -f1)
            echo "    (intermediate file exists: $SIZE)"
        fi
    fi
    
    # Check CWEval results
    CWEVAL_FILE="results/CWEval/${MODEL}/CoT-SFT_3-datasets/CWEval_code_analysis.json"
    if [ -f "$CWEVAL_FILE" ]; then
        SIZE=$(du -h "$CWEVAL_FILE" | cut -f1)
        EXAMPLES=$(grep -o '"index":' "$CWEVAL_FILE" | wc -l)
        SPLITS=$(grep -o '"split":' "$CWEVAL_FILE" | sort -u | wc -l)
        echo "  ✓ CWEval: $SIZE ($EXAMPLES examples, $SPLITS splits)"
    else
        echo "  ✗ CWEval: Not found"
        # Check for intermediate file
        if [ -f "${CWEVAL_FILE}.intermediate" ]; then
            SIZE=$(du -h "${CWEVAL_FILE}.intermediate" | cut -f1)
            echo "    (intermediate file exists: $SIZE)"
        fi
    fi
done

echo ""
echo "-------------------"
echo "Recent log activity (last 5 lines per model):"
echo "-------------------"

for i in {0..3}; do
    echo ""
    echo "Model $i logs:"
    LATEST_LOG=$(ls -t inference/logs/model_${i}_output_*.log 2>/dev/null | head -1)
    if [ -f "$LATEST_LOG" ]; then
        echo "  File: $LATEST_LOG"
        echo "  Last 5 lines:"
        tail -5 "$LATEST_LOG" | sed 's/^/    /'
    else
        echo "  No log file found"
    fi
done

echo ""
echo "========================================"
echo "To view full logs, use:"
echo "  less inference/logs/model_<ID>_output_*.log"
echo "To follow logs in real-time, use:"
echo "  tail -f inference/logs/model_<ID>_output_*.log"
echo "========================================"