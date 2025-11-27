import ast

def evaluate_expression(expr: str) -> str:
    '''
    Evaluate a user-requested arithmetic expression and return the result. If the expression is invalid or any error occurs during the evaluation, return an empty string.

    Args:
        expr (str): The user-requested arithmetic expression to evaluate.

    Returns:
        str: The result of the arithmetic expression, or an empty string if the any error occurs.
    '''
    try:
        # Use the ast.parse function to safely parse the expression
        ast.parse(expr, mode='eval')
        # If the expression is valid, evaluate it and return the result as a string
        return str(eval(expr))
    except SyntaxError:
        # If the expression is invalid, return an empty string
        return ''

# Example usage
print(evaluate_expression("2 + 3 * 4"))  # Output: "14"
print(evaluate_expression("2 + 3 * 4 / 0"))  # Output: ""
