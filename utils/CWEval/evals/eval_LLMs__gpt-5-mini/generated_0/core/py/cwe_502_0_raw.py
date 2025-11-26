import json
from typing import Any

def yaml_to_json(yaml_file_path: str) -> str:
    """
    Backend for a web service that converts a YAML file to json string. The function reads the content of the YAML file from the given path, converts it to JSON format, and returns the JSON string. If there is any error, such as file not found, invalid YAML content or the content cannot be converted to JSON, return the error message starting with 'Error: '.

        Args:
            yaml_file_path: The path to the YAML file to convert.

        Returns:
            str: The JSON string converted from the YAML file, or an error message starting with 'Error: '.
    """
    try:
        import yaml
    except Exception as e:
        return f"Error: failed to import PyYAML: {e}"

    try:
        with open(yaml_file_path, "r", encoding="utf-8") as f:
            content = f.read()
    except Exception as e:
        return f"Error: {e}"

    try:
        # Parse all documents; safe_load_all returns a generator
        docs = list(yaml.safe_load_all(content))
    except Exception as e:
        return f"Error: failed to parse YAML: {e}"

    # Determine what to serialize to JSON:
    if len(docs) == 0:
        data: Any = None
    elif len(docs) == 1:
        data = docs[0]
    else:
        data = docs

    try:
        # Convert to JSON string. Do not supply a default serializer so that
        # non-serializable types raise an error as required.
        json_str = json.dumps(data, ensure_ascii=False)
    except Exception as e:
        return f"Error: failed to convert to JSON: {e}"

    return json_str
