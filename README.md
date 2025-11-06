# SeCodePLT Unit Test Instructions

To execute the unit tests for a given `SecCodePLT_CoT_SFT_Results.json` file present in the `./results/CoT_SFT/<model-name>/` directory, follow these steps:

1. **Start the Server**:
```bash
python -m utils.SeCodePLT_updated.executor_docker.server
```
A server will start on `localhost:8666` by default. You can configure it to run on a different host or port by setting the `SERVER_PORT` environment variable present in the `utils/SeCodePLT_updated/.env` file. OR, by running the following command:
```bash
python -m executor_docker.server --port <desired-port>
```

2. **Run the Unit Tests**:
```bash
python utils/SeCodePLT_updated/python_eval_script.py --model_name <model-name>
```
Replace `<model-name>` with the actual name of the model whose results you want to test. `<model-name>` can only be one of the folders present in the `./results/CoT_SFT/` directory. This folder should contain the `SecCodePLT_CoT_SFT_Results.json` file.

# CWEval Unit Test Instructions

1. Convert the JSON results to CWEval format:
```bash
# Navigate to the root directory of the project
# the parent directory should point to the model results folder present in ./results/CWEval/*
# this should contain a file named CWEval_Results.json
python -m scripts.convert_cweval_json_to_eval \
--parent LLMs/gpt-4o
```

2. 
```bash
cd utils/CWEval

# Run Tests for python only
# eval_path is created in step 1 in the following format: evals/eval_<parent>__<model-name>
# run_tests will execute tests for only the specified language
python -m cweval.evaluate run_tests \
--eval_path evals/eval_LLMs__gpt-4o

# Now execute merge results
python -m cweval.evaluate \
_merge_results \
--eval_path evals/eval_LLMs__gpt-4o

python -m cweval.evaluate \
report_pass_at_k \
--eval_path evals/eval_deepseek-coder-7b__CoT-SFT_RLVR \
--k 1 \
--lang core/py/

### ONLY for running the full pipeline (all languages) ###
# pipeline will execute tests for all languages
python cweval/evaluate.py pipeline \
--eval_path evals/eval_LLMs__gpt-4o \
--langs py
```