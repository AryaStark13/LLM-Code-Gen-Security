import yaml

def yaml_to_json(yaml_file_path: str) -> str:
    try:
        with open(yaml_file_path, 'r') as file:
            data = yaml.safe_load(file)
    except FileNotFoundError:
        return 'Error: File not found.'
    except yaml.YAMLError:
        return 'Error: Invalid YAML content.'

    try:
        return json.dumps(data)
    except Exception as e:
        return f'Error: Failed to convert YAML to JSON. {str(e)}'
