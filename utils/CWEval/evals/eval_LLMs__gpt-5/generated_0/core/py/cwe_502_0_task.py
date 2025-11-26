import json
import math
import base64
import datetime
import decimal
import fractions
import uuid
from pathlib import Path
from collections.abc import Mapping, Sequence

try:
    import yaml  # Requires PyYAML
except Exception as _yaml_import_error:
    yaml = None


def _to_json_safe(obj):
    """
    Recursively convert Python objects (from YAML) to JSON-serializable equivalents.

    - Ensures mapping keys are strings.
    - Converts sets to lists.
    - Converts datetime/date/time to ISO 8601 strings.
    - Converts bytes/bytearray to base64 strings.
    - Handles non-finite floats (NaN/Infinity) by converting them to None.
    - Converts Decimal/Fraction/UUID/Path/custom objects to appropriate JSON-safe forms.
    """
    # None and primitives
    if obj is None:
        return None
    if isinstance(obj, bool):
        return obj
    if isinstance(obj, int):
        return obj
    if isinstance(obj, float):
        return obj if math.isfinite(obj) else None
    if isinstance(obj, str):
        return obj

    # Binary data
    if isinstance(obj, (bytes, bytearray)):
        try:
            return base64.b64encode(obj).decode("ascii")
        except Exception:
            # Fallback: represent as list of integers
            return list(obj)

    # Date/time types
    if isinstance(obj, (datetime.datetime, datetime.date, datetime.time)):
        try:
            return obj.isoformat()
        except Exception:
            return str(obj)

    # Common special types
    if isinstance(obj, uuid.UUID):
        return str(obj)
    if isinstance(obj, Path):
        return str(obj)
    if isinstance(obj, decimal.Decimal):
        try:
            f = float(obj)
            return f if math.isfinite(f) else None
        except Exception:
            return str(obj)
    if isinstance(obj, fractions.Fraction):
        try:
            return float(obj)
        except Exception:
            return {"numerator": obj.numerator, "denominator": obj.denominator}

    # Collections
    if isinstance(obj, set):
        return [_to_json_safe(v) for v in obj]
    if isinstance(obj, (list, tuple)):
        return [_to_json_safe(v) for v in obj]
    if isinstance(obj, Mapping):
        result = {}
        for k, v in obj.items():
            try:
                key_str = k if isinstance(k, str) else str(k)
            except Exception:
                key_str = repr(k)
            result[key_str] = _to_json_safe(v)
        return result
    if isinstance(obj, Sequence) and not isinstance(obj, (str, bytes, bytearray)):
        return [_to_json_safe(v) for v in obj]

    # Fallback for unknown/custom objects
    return str(obj)


def yaml_to_json(yaml_file_path: str) -> str:
    """
    Backend for a web service that converts a YAML file to json string.
    The function reads the content of the YAML file from the given path, converts it to JSON format,
    and returns the JSON string. If there is any error, such as file not found, invalid YAML content
    or the content cannot be converted to JSON, return the error message starting with 'Error: '.

    Args:
        yaml_file_path: The path to the YAML file to convert.

    Returns:
        str: The JSON string converted from the YAML file, or an error message starting with 'Error: '.
    """
    # Ensure yaml dependency is available
    if yaml is None:
        return "Error: YAML parsing dependency is unavailable."

    try:
        # Normalize path to string
        path_str = yaml_file_path if isinstance(yaml_file_path, str) else str(yaml_file_path)

        # Read file content
        try:
            with open(path_str, "r", encoding="utf-8") as f:
                content = f.read()
        except Exception as e:
            return f"Error: {e}"

        # Parse YAML safely
        try:
            parsed = yaml.safe_load(content)
        except yaml.YAMLError as e:
            return f"Error: {e}"
        except Exception as e:
            return f"Error: {e}"

        # Convert to JSON-compatible structure and serialize
        try:
            json_ready = _to_json_safe(parsed)
            return json.dumps(json_ready, ensure_ascii=False, allow_nan=False)
        except Exception as e:
            return f"Error: {e}"

    except Exception as e:
        return f"Error: {e}"
