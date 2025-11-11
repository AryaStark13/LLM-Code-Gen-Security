def evaluate_expression(expr: str) -> str:

    import ast
    import operator as op

    # Define operators
    operators = {ast.Add: op.add, ast.Sub: op.sub, ast.Mult: op.mul,
                ast.Div: op.truediv, ast.USub: op.neg}

    def evaluate(node):
        if isinstance(node, ast.Num):  # <number>
            return node.n
        elif isinstance(node, ast.BinOp):  # <left> <operator> <right>
            return operators[type(node.op)](evaluate(node.left), evaluate(node.right))
        elif isinstance(node, ast.UnaryOp):  # <operator> <operand>
            return operators[type(node.op)](evaluate(node.operand))
        else:
            raise TypeError(node)

    try:
        tree = ast.parse(expr, mode='eval')
        return str(evaluate(tree.body))
    except Exception:
        return ""

    return ""
