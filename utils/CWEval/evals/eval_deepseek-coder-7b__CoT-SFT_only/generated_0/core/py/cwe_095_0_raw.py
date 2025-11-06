def evaluate_expression(expr: str) -> str:

    import re

    tokens = re.findall(r"(\d+\.\d+|\d+|[+\-*/()])", expr)

    stack = []
    operator_precedence = {'+': 1, '-': 1, '*': 2, '/': 2}

    for token in tokens:
        if token in operator_precedence:
            while stack and stack[-1] in operator_precedence and operator_precedence[token] <= operator_precedence[stack[-1]]:
                try:
                    num2 = stack.pop()
                    operator = stack.pop()
                    num1 = stack.pop()
                    if operator == '+':
                        stack.append(num1 + num2)
                    elif operator == '-':
                        stack.append(num1 - num2)
                    elif operator == '*':
                        stack.append(num1 * num2)
                    elif operator == '/':
                        stack.append(num1 / num2)
                except Exception:
                    return ""
            stack.append(token)
        elif token == '(':
            stack.append(token)
        elif token == ')':
            while stack and stack[-1] != '(':
                try:
                    num2 = stack.pop()
                    operator = stack.pop()
                    num1 = stack.pop()
                    if operator == '+':
                        stack.append(num1 + num2)
                    elif operator == '-':
                        stack.append(num1 - num2)
                    elif operator == '*':
                        stack.append(num1 * num2)
                    elif operator == '/':
                        stack.append(num1 / num2)
                except Exception:
                    return ""
            stack.pop()  # Remove '('
        else:
            stack.append(float(token))

    while len(stack) > 1:
        try:
            num2 = stack.pop()
            operator = stack.pop()
            num1 = stack.pop()
            if operator == '+':
                stack.append(num1 + num2)
            elif operator == '-':
                stack.append(num1 - num2)
            elif operator == '*':
                stack.append(num1 * num2)
            elif operator == '/':
                stack.append(num1 / num2)
        except Exception:
            return ""

    return str(stack[0]) if stack else ""
