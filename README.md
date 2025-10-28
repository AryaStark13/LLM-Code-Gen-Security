To execute the unit tests for a given `SecCodePLT_CoT_SFT_Results.json` file present in the `./results/CoT_SFT/<model-name>/` directory, follow these steps:

1. **Start the Server**:
```bash
.venv/bin/python -m utils.SeCodePLT_updated.executor_docker.server
```
A server will start on `localhost:8666` by default. You can configure it to run on a different host or port by setting the `SERVER_PORT` environment variable present in the `utils/SeCodePLT_updated/.env` file. OR, by running the following command:
```bash
.venv/bin/python -m executor_docker.server --port <desired-port>
```

2. **Run the Unit Tests**:
```bash
.venv/bin/python utils/SeCodePLT_updated/python_eval_script.py --model_name <model-name>
```
Replace `<model-name>` with the actual name of the model whose results you want to test. `<model-name>` can only be one of the folders present in the `./results/CoT_SFT/` directory. This folder should contain the `SecCodePLT_CoT_SFT_Results.json` file.

