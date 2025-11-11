import yaml
import json

def yaml_to_json(yaml_file_path: str) -> str:
    try:
        with open(yaml_file_path, 'r') as file:
            yaml_content = file.read()
    except FileNotFoundError:
        return "Error: File not found."
    except Exception as e:
        return f"Error: {str(e)}"

    try:
        yaml_data = yaml.safe_load(yaml_content)
    except yaml.YAMLError as e:
        return f"Error: {str(e)}"

    try:
        json_string = json.dumps(yaml_data)
        return json_string
    except Exception as e:
        return f"Error: {str(e)}"
