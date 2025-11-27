import os
import json
import yaml
from yaml import Loader

def yaml_to_json(yaml_file_path: str) -> str:
    if not os.path.exists(yaml_file_path):
        return "Error: File not found"

    try:
        with open(yaml_file_path, "r") as f:
            yaml_data = yaml.load(f, Loader=Loader)
    except Exception as e:
        return f"Error: {str(e)}"

    try:
        json_string = json.dumps(yaml_data)
    except Exception as e:
        return f"Error: {str(e)}"

    return json_string
