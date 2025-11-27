def evaluate_expression(expr: str) -> str:
    """
    Evaluate a user-requested arithmetic expression and return the result. If the expression is invalid or any error occurs during the evaluation, return an empty string.

    Args:
        expr (str): The user-requested arithmetic expression to evaluate.

    Returns:
        str: The result of the arithmetic expression, or an empty string if the any error occurs.
    """

    # Check if the input is a string
    if not isinstance(expr, str):
        return ""

    # Check if the input is a valid arithmetic expression
    if not expr.isdigit() and not expr.replace("-", "").replace("+", "").replace("*", "").replace("/", "").isdigit():
        return ""

    # Evaluate the expression
    try:
        result = eval(expr)
        return str(result)
    except Exception:
        return ""
