def evaluate_expression(expr: str) -> str:

    try:
        result = eval(expr, {"__builtins__": {}}, {})
        return str(result)
    except Exception:
        return ""
