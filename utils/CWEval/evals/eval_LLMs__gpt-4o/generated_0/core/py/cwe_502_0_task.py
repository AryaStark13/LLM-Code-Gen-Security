import yaml
import json

def yaml_to_json(yaml_file_path: str) -> str:
    try:
        # Read the YAML file
        with open(yaml_file_path, 'r') as file:
            yaml_content = file.read()
        
        # Parse the YAML content
        try:
            parsed_data = yaml.safe_load(yaml_content)
        except yaml.YAMLError as e:
            return f"Error: Invalid YAML content - {str(e)}"
        
        # Convert to JSON
        try:
            json_string = json.dumps(parsed_data)
            return json_string
        except (TypeError, ValueError) as e:
            return f"Error: Cannot convert to JSON - {str(e)}"
    
    except FileNotFoundError:
        return "Error: File not found"
    except IOError as e:
        return f"Error: IOError - {str(e)}"
