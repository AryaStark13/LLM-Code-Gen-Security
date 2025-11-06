def evaluate_expression(expr: str) -> str:
    try:
        return str(eval(expr))
    except:
        return ""
