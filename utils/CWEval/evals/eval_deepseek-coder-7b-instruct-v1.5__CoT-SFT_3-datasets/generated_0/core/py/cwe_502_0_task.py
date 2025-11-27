import json
import yaml
from yaml import Loader

def yaml_to_json(yaml_file_path: str) -> str:
    try:
        with open(yaml_file_path, 'r') as file:
            yaml_content = file.read()
            data = yaml.load(yaml_content, Loader=Loader)
            json_data = json.dumps(data)
            return json_data
    except FileNotFoundError:
        return 'Error: File not found.'
    except yaml.YAMLError as e:
        return 'Error: Invalid YAML content.'
    except Exception as e:
        return 'Error: Unable to convert YAML to JSON.'
