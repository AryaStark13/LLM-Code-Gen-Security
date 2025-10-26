#!/bin/bash

# Setup
source .venv/bin/activate

# This script runs the functional tests for a given model results
# Assumes a results file named SecCodePLT_CoT_SFT_Results.json is present in results/{MODEL_NAME}/

# MODEL_NAME="deepseek-coder-1.3b-instruct-seccodeplt-cot-sft-5-epochs-with-tag-instr"
MODEL_NAME=$1

# Step 1: Convert the JSON results to the required JSONL format
# Format: {"task_id": "ID of the task", "solution": "the solution text"}

python3 utils/results_json_to_jsonl.py --model_name $MODEL_NAME

# Resulting JSONL file is stored at: results/{MODEL_NAME}/SecCodePLT_CoT_SFT_Results.jsonl

if [ $? -ne 0 ]; then
    echo "Error converting JSON to JSONL format."
    exit 1
fi

echo "Step 1: Successfully converted JSON to JSONL format."

Step 2: Run the functional tests using the converted JSONL file
bash script is called run.sh located in "utils/SecCodePLT+_func_tests/run.sh"
It takes the model name as an argument

cd utils/SecCodePLT+_func_tests/
bash run.sh $MODEL_NAME

if [ $? -ne 0 ]; then
    echo "Error running functional tests."
    exit 1
fi

# results should be saved to utils/SecCodePLT+_func_tests/data/results/${MODEL_NAME}
echo "Step 2: Functional tests completed successfully. Change directory back to root."

cd ../../

echo "Copying results to main results directory..."

# Step 3: Copy the results back to the main results directory
cp utils/SecCodePLT+_func_tests/data/results/${MODEL_NAME}/SecCodePLT+_func_tests_results.json results/CoT_SFT/${MODEL_NAME}/SecCodePLT+_func_tests_results.json
cp utils/SecCodePLT+_func_tests/data/summary/${MODEL_NAME}/SecCodePLT+_func_tests_summary.json results/CoT_SFT/${MODEL_NAME}/SecCodePLT+_func_tests_summary.json

if [ $? -ne 0 ]; then
    echo "Error copying results to main results directory."
    exit 1
fi

echo "Step 3: Results copied successfully to results/CoT_SFT/${MODEL_NAME}/"

# Step 4: Run analysis on the functional test results

python3 utils/get_func_test_analysis.py --model_name $MODEL_NAME

if [ $? -ne 0 ]; then
    echo "Error running functional test analysis."
    exit 1
fi

echo "Step 4: Functional test analysis completed successfully. Results are in results/CoT_SFT/${MODEL_NAME}/SecCodePLT+_func_tests_results_analysis_report.json" and "results/CoT_SFT/${MODEL_NAME}/SecCodePLT+_code_analysis.json"

echo "All steps completed successfully."