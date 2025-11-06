def evaluate_expression(expr: str) -> str:
    try:
        # Evaluate the expression using eval
        result = eval(expr)
        # Convert the result to string and return
        return str(result)
    except Exception:
        # Return an empty string if any error occurs
        return ""
