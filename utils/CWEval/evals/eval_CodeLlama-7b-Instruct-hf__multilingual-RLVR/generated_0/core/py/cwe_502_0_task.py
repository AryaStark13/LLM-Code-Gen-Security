import json

import yaml
from yaml import Loader


def yaml_to_json(yaml_file_path: str) -> str:
    try:
        with open(yaml_file_path, 'r') as file:
            yaml_content = file.read()
            json_content = json.dumps(yaml.load(yaml_content, Loader=Loader))
            return json_content
    except Exception as e:
        return f"Error: {str(e)}"
