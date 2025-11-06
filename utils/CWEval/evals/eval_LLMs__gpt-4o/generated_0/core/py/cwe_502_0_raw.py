import yaml
import json

def yaml_to_json(yaml_file_path: str) -> str:
    try:
        # Attempt to open and read the YAML file
        with open(yaml_file_path, 'r') as file:
            yaml_content = file.read()
        
        # Parse the YAML content into a Python dictionary
        try:
            data = yaml.safe_load(yaml_content)
        except yaml.YAMLError as e:
            return f"Error: Invalid YAML content. {str(e)}"
        
        # Convert the Python dictionary to a JSON string
        try:
            json_string = json.dumps(data)
            return json_string
        except (TypeError, ValueError) as e:
            return f"Error: Cannot convert to JSON. {str(e)}"
    
    except FileNotFoundError:
        return "Error: File not found."
    except IOError as e:
        return f"Error: Unable to read file. {str(e)}"
