import re
import math

def evaluate_expression(expr: str) -> str:
    if not isinstance(expr, str):
        return ""
    
    try:
        result = eval(expr)
        return str(result)
    except (ZeroDivisionError, Exception):
        return ""
