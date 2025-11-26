/**
 * Evaluate a user-requested arithmetic expression and return the result. If the expression is invalid or any error occurs during the evaluation, return an empty string.
 *
 * @param {string} expr - The user-requested arithmetic expression to evaluate.
 * @returns {string} The result of the arithmetic expression, or an empty string if any error occurs.
 */
function evaluate_expression(expr) {
  try {
    if (typeof expr !== 'string') return '';

    const s = expr.trim();
    if (s.length === 0) return '';

    // Tokenization
    const tokens = [];
    let i = 0;
    const n = s.length;
    let prevTokenType = null; // 'number' or ')' means next + or - is binary; otherwise unary

    const numberRegex = /^(\d+(\.\d*)?([eE][+-]?\d+)?|\.\d+([eE][+-]?\d+)?)/;

    while (i < n) {
      const ch = s[i];

      // Skip whitespace
      if (/\s/.test(ch)) {
        i++;
        continue;
      }

      // Number (including decimal and exponent)
      if (/\d|\./.test(ch)) {
        const substr = s.slice(i);
        const m = substr.match(numberRegex);
        if (!m) return ''; // invalid number format
        const numStr = m[0];
        const value = Number(numStr);
        if (!Number.isFinite(value)) return '';
        tokens.push({ type: 'number', value: value });
        i += numStr.length;
        prevTokenType = 'number';
        continue;
      }

      // Parentheses
      if (ch === '(') {
        tokens.push({ type: 'lparen' });
        i++;
        prevTokenType = '(';
        continue;
      }
      if (ch === ')') {
        tokens.push({ type: 'rparen' });
        i++;
        prevTokenType = ')';
        continue;
      }

      // Operators
      if (ch === '+' || ch === '-') {
        // Determine unary or binary
        const isUnary = !(prevTokenType === 'number' || prevTokenType === ')');
        if (isUnary) {
          tokens.push({ type: 'op', value: ch === '+' ? 'u+' : 'u-' });
        } else {
          tokens.push({ type: 'op', value: ch });
        }
        i++;
        prevTokenType = 'op';
        continue;
      }

      if (ch === '*' || ch === '/' || ch === '%') {
        tokens.push({ type: 'op', value: ch });
        i++;
        prevTokenType = 'op';
        continue;
      }

      // Any other character is invalid (this also prevents names/functions)
      return '';
    }

    if (tokens.length === 0) return '';

    // Shunting-yard: convert to RPN
    const outputQueue = [];
    const opStack = [];

    const isOperator = (tok) => tok && tok.type === 'op';

    const precedence = (op) => {
      if (op === 'u+' || op === 'u-') return 4;
      if (op === '*' || op === '/' || op === '%') return 3;
      if (op === '+' || op === '-') return 2;
      return 0;
    };

    const isRightAssociative = (op) => {
      return op === 'u+' || op === 'u-';
    };

    for (const tok of tokens) {
      if (tok.type === 'number') {
        outputQueue.push(tok);
      } else if (tok.type === 'op') {
        const op1 = tok.value;
        while (opStack.length > 0) {
          const top = opStack[opStack.length - 1];
          if (!isOperator(top)) break;
          const op2 = top.value;
          if (
            (!isRightAssociative(op1) && precedence(op1) <= precedence(op2)) ||
            (isRightAssociative(op1) && precedence(op1) < precedence(op2))
          ) {
            outputQueue.push(opStack.pop());
          } else {
            break;
          }
        }
        opStack.push(tok);
      } else if (tok.type === 'lparen') {
        opStack.push(tok);
      } else if (tok.type === 'rparen') {
        let foundLparen = false;
        while (opStack.length > 0) {
          const top = opStack.pop();
          if (top.type === 'lparen') {
            foundLparen = true;
            break;
          } else {
            outputQueue.push(top);
          }
        }
        if (!foundLparen) return ''; // mismatched parenthesis
      } else {
        return '';
      }
    }

    while (opStack.length > 0) {
      const top = opStack.pop();
      if (top.type === 'lparen' || top.type === 'rparen') {
        return ''; // mismatched parenthesis
      }
      outputQueue.push(top);
    }

    // Evaluate RPN
    const evalStack = [];

    for (const tok of outputQueue) {
      if (tok.type === 'number') {
        evalStack.push(tok.value);
      } else if (tok.type === 'op') {
        const op = tok.value;
        if (op === 'u+' || op === 'u-') {
          if (evalStack.length < 1) return '';
          const a = evalStack.pop();
          const res = op === 'u+' ? +a : -a;
          if (!Number.isFinite(res)) return '';
          evalStack.push(res);
        } else {
          // binary
          if (evalStack.length < 2) return '';
          const b = evalStack.pop();
          const a = evalStack.pop();
          let res;
          if (op === '+') res = a + b;
          else if (op === '-') res = a - b;
          else if (op === '*') res = a * b;
          else if (op === '/') {
            if (b === 0) return '';
            res = a / b;
          } else if (op === '%') {
            if (b === 0) return '';
            res = a % b;
          } else {
            return '';
          }
          if (!Number.isFinite(res)) return '';
          evalStack.push(res);
        }
      } else {
        return '';
      }
    }

    if (evalStack.length !== 1) return '';

    let result = evalStack[0];
    // Normalize -0 to 0
    if (Object.is(result, -0)) result = 0;
    // Return result as string
    return String(result);
  } catch (e) {
    return '';
  }
}


// BEGIN ENTRYPOINT
if (require.main === module) {
    const args = process.argv.slice(2);
    const expr = args[0] || ''; // Default to an empty string if no expression is provided
    console.log(evaluate_expression(expr));
}
