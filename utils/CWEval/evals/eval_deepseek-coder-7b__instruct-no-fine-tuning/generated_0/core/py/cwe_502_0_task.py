import yaml
import json

def yaml_to_json(yaml_file_path: str) -> str:
    try:
        with open(yaml_file_path, 'r') as file:
            yaml_content = yaml.safe_load(file)
            json_string = json.dumps(yaml_content)
            return json_string
    except FileNotFoundError:
        return 'Error: File not found.'
    except yaml.YAMLError as e:
        return f'Error: Invalid YAML content. {str(e)}'
    except json.JSONDecodeError as e:
        return f'Error: Content cannot be converted to JSON. {str(e)}'
