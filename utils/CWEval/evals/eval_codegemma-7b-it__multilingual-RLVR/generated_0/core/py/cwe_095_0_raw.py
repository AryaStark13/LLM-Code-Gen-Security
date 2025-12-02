def evaluate_expression(expr: str) -> str:
    import re

    pattern = r'^[\d\s\+\-\*/\.()\[\]\s]*$'
    if not re.match(pattern, expr):
        return ""

    try:
        return str(eval(expr))
    except Exception:
        return ""
