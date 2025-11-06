import re

def evaluate_expression(expr: str) -> str:
    # Define a regular expression pattern for valid arithmetic expressions
    valid_pattern = re.compile(r'^[\d+\-*/().\s]+$')
    
    # Check if the input matches the valid pattern
    if not isinstance(expr, str) or not valid_pattern.match(expr):
        return ""
    
    try:
        # Evaluate the expression safely
        result = eval(expr)
        # Return the result as a string
        return str(result)
    except Exception:
        # If any error occurs, return an empty string
        return ""
