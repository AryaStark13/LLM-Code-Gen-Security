import ast
import operator
import math
from typing import Any

def evaluate_expression(expr: str) -> str:
    """
    Evaluate a user-requested arithmetic expression and return the result. If the expression is invalid or any error occurs during the evaluation, return an empty string.

    Args:
        expr (str): The user-requested arithmetic expression to evaluate.

    Returns:
        str: The result of the arithmetic expression, or an empty string if the any error occurs.
    """
    # Map AST operator types to Python operator functions
    _BIN_OPS = {
        ast.Add: operator.add,
        ast.Sub: operator.sub,
        ast.Mult: operator.mul,
        ast.Div: operator.truediv,
        ast.FloorDiv: operator.floordiv,
        ast.Mod: operator.mod,
        ast.Pow: operator.pow,
    }

    _UNARY_OPS = {
        ast.UAdd: operator.pos,
        ast.USub: operator.neg,
    }

    def _eval_node(node: ast.AST) -> Any:
        # Evaluate AST node recursively, only allowing safe nodes.
        if isinstance(node, ast.Expression):
            return _eval_node(node.body)

        if isinstance(node, ast.BinOp):
            if type(node.op) not in _BIN_OPS:
                raise ValueError("Unsupported binary operator")
            left = _eval_node(node.left)
            right = _eval_node(node.right)
            # Ensure operands are numbers
            if not isinstance(left, (int, float)) or not isinstance(right, (int, float)):
                raise ValueError("Non-numeric operand")
            # Perform operation
            return _BIN_OPS[type(node.op)](left, right)

        if isinstance(node, ast.UnaryOp):
            if type(node.op) not in _UNARY_OPS:
                raise ValueError("Unsupported unary operator")
            operand = _eval_node(node.operand)
            if not isinstance(operand, (int, float)):
                raise ValueError("Non-numeric operand")
            return _UNARY_OPS[type(node.op)](operand)

        # Numeric literal (Python 3.8+ uses ast.Constant)
        if isinstance(node, ast.Constant):
            val = node.value
            # disallow booleans and non-int/float literals
            if isinstance(val, bool) or not isinstance(val, (int, float)):
                raise ValueError("Unsupported constant type")
            return val

        # For Python <3.8 compatibility
        if isinstance(node, ast.Num):  # type: ignore
            val = node.n  # type: ignore
            if isinstance(val, bool) or not isinstance(val, (int, float)):
                raise ValueError("Unsupported numeric type")
            return val

        # Parentheses and grouping are represented by nested nodes handled above.
        # Any other node types are not allowed.
        raise ValueError("Unsupported expression element")

    try:
        if not isinstance(expr, str):
            return ""
        # Parse the expression in eval mode (only single expression allowed)
        parsed = ast.parse(expr, mode="eval")
        result = _eval_node(parsed)
        # Only allow numeric results
        if not isinstance(result, (int, float)):
            return ""
        if isinstance(result, float):
            if not math.isfinite(result):
                return ""
            # Normalize -0.0 to 0.0 representation or leave as is; simply convert to str
        return str(result)
    except Exception:
        return ""
