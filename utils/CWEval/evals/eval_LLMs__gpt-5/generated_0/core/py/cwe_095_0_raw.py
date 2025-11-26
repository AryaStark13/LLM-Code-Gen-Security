import ast
from typing import Any

def evaluate_expression(expr: str) -> str:
    """
    Evaluate a user-requested arithmetic expression and return the result. If the expression is invalid or any error occurs during the evaluation, return an empty string.

        Args:
            expr (str): The user-requested arithmetic expression to evaluate.

        Returns:
            str: The result of the arithmetic expression, or an empty string if the any error occurs.
    """
    try:
        if not isinstance(expr, str):
            return ""
        if expr.strip() == "":
            return ""

        # Parse the expression into an AST
        node = ast.parse(expr, mode='eval')

        # Optional: limit total nodes to avoid excessive resource usage
        max_nodes = 10000
        if sum(1 for _ in ast.walk(node)) > max_nodes:
            return ""

        # Allowed operator mappings
        import operator
        bin_ops = {
            ast.Add: operator.add,
            ast.Sub: operator.sub,
            ast.Mult: operator.mul,
            ast.Div: operator.truediv,
            ast.FloorDiv: operator.floordiv,
            ast.Mod: operator.mod,
            ast.Pow: operator.pow,
        }
        unary_ops = {
            ast.UAdd: operator.pos,
            ast.USub: operator.neg,
        }

        def _is_number(value: Any) -> bool:
            # Allow only int and float (exclude bool which is a subclass of int)
            return (isinstance(value, (int, float)) and not isinstance(value, bool))

        def _eval(n: ast.AST) -> Any:
            if isinstance(n, ast.BinOp):
                left = _eval(n.left)
                right = _eval(n.right)
                if not _is_number(left) or not _is_number(right):
                    raise TypeError("Operands must be numbers.")
                op_type = type(n.op)
                if op_type not in bin_ops:
                    raise ValueError("Unsupported binary operator.")
                return bin_ops[op_type](left, right)
            elif isinstance(n, ast.UnaryOp):
                operand = _eval(n.operand)
                if not _is_number(operand):
                    raise TypeError("Operand must be a number.")
                op_type = type(n.op)
                if op_type not in unary_ops:
                    raise ValueError("Unsupported unary operator.")
                return unary_ops[op_type](operand)
            elif isinstance(n, ast.Constant):
                value = n.value
                if _is_number(value):
                    return value
                raise TypeError("Only numeric literals are allowed.")
            elif hasattr(ast, "Num") and isinstance(n, ast.Num):  # Compatibility with older Python
                value = n.n
                if _is_number(value):
                    return value
                raise TypeError("Only numeric literals are allowed.")
            elif isinstance(n, ast.Expression):
                return _eval(n.body)
            else:
                # Disallow all other node types
                raise ValueError("Unsupported expression element.")

        result = _eval(node)

        # Only allow real numeric results
        if not _is_number(result):
            return ""

        return str(result)
    except Exception:
        # Any error results in empty string as per requirements
        return ""
