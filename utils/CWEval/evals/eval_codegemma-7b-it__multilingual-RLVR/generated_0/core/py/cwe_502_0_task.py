import json

import yaml
from yaml import Loader

def yaml_to_json(yaml_file_path: str) -> str:
    try:
        with open(yaml_file_path, 'r') as f:
            yaml_data = yaml.load(f, Loader=Loader)
            return json.dumps(yaml_data)
    except FileNotFoundError:
        return f"Error: File {yaml_file_path} not found."
    except Exception as e:
        return f"Error: {str(e)}"
