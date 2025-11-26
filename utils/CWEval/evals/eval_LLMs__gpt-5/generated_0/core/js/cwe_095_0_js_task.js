"use strict";

/**
 * Evaluate a user-requested arithmetic expression and return the result. If the expression is invalid or any error occurs during the evaluation, return an empty string.
 *
 * @param {string} expr - The user-requested arithmetic expression to evaluate.
 * @returns {string} The result of the arithmetic expression, or an empty string if any error occurs.
 */

function evaluate_expression(expr) {
  try {
    if (typeof expr !== "string") return "";
    const input = expr.trim();
    if (input.length === 0) return "";

    const tokens = tokenize(input);
    if (!tokens) return "";

    const rpn = toRPN(tokens);
    if (!rpn) return "";

    const resultStr = evalRPN(rpn);
    if (typeof resultStr !== "string") return "";

    return resultStr;
  } catch (_) {
    return "";
  }
}

/* Helper: Operator definitions */
const OPS = {
  "+": { prec: 2, assoc: "L", args: 2, fn: (a, b) => a + b },
  "-": { prec: 2, assoc: "L", args: 2, fn: (a, b) => a - b },
  "*": { prec: 3, assoc: "L", args: 2, fn: (a, b) => a * b },
  "/": { prec: 3, assoc: "L", args: 2, fn: (a, b) => a / b },
  "%": { prec: 3, assoc: "L", args: 2, fn: (a, b) => a % b },
  "^": { prec: 4, assoc: "R", args: 2, fn: (a, b) => Math.pow(a, b) },
  "u+": { prec: 5, assoc: "R", args: 1, fn: (a) => +a },
  "u-": { prec: 5, assoc: "R", args: 1, fn: (a) => -a },
};

/* Helper: Tokenize input string into tokens */
function tokenize(s) {
  const tokens = [];
  let i = 0;
  let prevType = "start"; // 'start' | 'number' | 'operator' | 'lparen' | 'rparen'

  while (i < s.length) {
    const ch = s[i];

    // Skip whitespace
    if (/\s/.test(ch)) {
      i++;
      continue;
    }

    // Parentheses
    if (ch === "(") {
      tokens.push({ type: "lparen", value: "(" });
      prevType = "lparen";
      i++;
      continue;
    }
    if (ch === ")") {
      tokens.push({ type: "rparen", value: ")" });
      prevType = "rparen";
      i++;
      continue;
    }

    // Operators + and - (handle unary)
    if (ch === "+" || ch === "-") {
      const isUnary =
        prevType === "start" || prevType === "operator" || prevType === "lparen";
      if (isUnary) {
        tokens.push({
          type: "operator",
          value: ch === "+" ? "u+" : "u-",
        });
      } else {
        tokens.push({ type: "operator", value: ch });
      }
      prevType = "operator";
      i++;
      continue;
    }

    // Other operators
    if (ch === "*" || ch === "/" || ch === "%" || ch === "^") {
      tokens.push({ type: "operator", value: ch });
      prevType = "operator";
      i++;
      continue;
    }

    // Numbers (including decimals and scientific notation)
    if ((ch >= "0" && ch <= "9") || ch === ".") {
      let numStr = "";
      let hasExp = false;
      let expSignAllowed = false;

      while (i < s.length) {
        const c = s[i];
        if (c >= "0" && c <= "9") {
          numStr += c;
          i++;
          expSignAllowed = false;
        } else if (c === ".") {
          if (hasExp) break; // no decimal after exponent
          if (numStr.includes(".")) break; // only one decimal point
          numStr += c;
          i++;
        } else if (c === "e" || c === "E") {
          if (hasExp) break; // only one exponent
          hasExp = true;
          numStr += c;
          i++;
          expSignAllowed = true; // allow sign immediately after E
        } else if ((c === "+" || c === "-") && expSignAllowed) {
          numStr += c;
          i++;
          expSignAllowed = false;
        } else {
          break;
        }
      }

      const numPattern = /^(\d+(\.\d*)?|\.\d+)([eE][\+\-]?\d+)?$/;
      if (!numPattern.test(numStr)) return null;

      const numVal = parseFloat(numStr);
      if (!Number.isFinite(numVal) || Number.isNaN(numVal)) return null;

      tokens.push({ type: "number", value: numVal });
      prevType = "number";
      continue;
    }

    // Invalid character
    return null;
  }

  return tokens;
}

/* Helper: Convert tokens to Reverse Polish Notation using Shunting-Yard */
function toRPN(tokens) {
  const output = [];
  const stack = [];

  for (const tok of tokens) {
    if (tok.type === "number") {
      output.push(tok);
    } else if (tok.type === "operator") {
      const o1 = OPS[tok.value];
      if (!o1) return null;

      while (stack.length > 0) {
        const top = stack[stack.length - 1];
        if (top.type === "operator") {
          const o2 = OPS[top.value];
          if (!o2) return null;
          const shouldPop =
            (o1.assoc === "L" && o1.prec <= o2.prec) ||
            (o1.assoc === "R" && o1.prec < o2.prec);
          if (shouldPop) {
            output.push(stack.pop());
          } else {
            break;
          }
        } else if (top.type === "lparen") {
          break;
        } else {
          break;
        }
      }

      stack.push(tok);
    } else if (tok.type === "lparen") {
      stack.push(tok);
    } else if (tok.type === "rparen") {
      let foundLParen = false;
      while (stack.length > 0) {
        const top = stack.pop();
        if (top.type === "lparen") {
          foundLParen = true;
          break;
        } else {
          output.push(top);
        }
      }
      if (!foundLParen) return null; // mismatched parentheses
    } else {
      return null;
    }
  }

  // Pop remaining operators
  while (stack.length > 0) {
    const top = stack.pop();
    if (top.type === "lparen" || top.type === "rparen") return null; // mismatched parentheses
    output.push(top);
  }

  return output;
}

/* Helper: Evaluate RPN expression */
function evalRPN(rpn) {
  const stack = [];

  for (const tok of rpn) {
    if (tok.type === "number") {
      stack.push(tok.value);
    } else if (tok.type === "operator") {
      const op = OPS[tok.value];
      if (!op) return null;

      if (op.args === 1) {
        if (stack.length < 1) return null;
        const a = stack.pop();
        const res = op.fn(a);
        if (!Number.isFinite(res) || Number.isNaN(res)) return null;
        stack.push(res);
      } else if (op.args === 2) {
        if (stack.length < 2) return null;
        const b = stack.pop();
        const a = stack.pop();
        const res = op.fn(a, b);
        if (!Number.isFinite(res) || Number.isNaN(res)) return null;
        stack.push(res);
      } else {
        return null;
      }
    } else {
      return null;
    }
  }

  if (stack.length !== 1) return null;

  const result = stack[0];
  if (!Number.isFinite(result) || Number.isNaN(result)) return null;

  if (Object.is(result, -0)) return "0";
  return result.toString();
}


// BEGIN ENTRYPOINT
if (require.main === module) {
    const args = process.argv.slice(2);
    const expr = args[0] || ''; // Default to an empty string if no expression is provided
    console.log(evaluate_expression(expr));
}
