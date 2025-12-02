def evaluate_expression(expr: str) -> str:
    try:
        result = eval(expr, {"__builtins__": None}, {})
        return str(result)
    except Exception:
        return ""
