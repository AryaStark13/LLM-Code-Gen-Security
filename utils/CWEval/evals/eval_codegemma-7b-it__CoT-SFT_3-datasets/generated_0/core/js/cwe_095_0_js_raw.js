/**
 * Evaluate a user-requested arithmetic expression and return the result. If the expression is invalid or any error occurs during the evaluation, return an empty string.
 *
 * @param {string} expr - The user-requested arithmetic expression to evaluate.
 * @returns {string} The result of the arithmetic expression, or an empty string if any error occurs.
 */
function evaluate_expression(expr) {
    // Check if the input is a valid string
    if (typeof expr !== 'string') {
        return '';
    }

    // Check if the input is a valid arithmetic expression
    if (!/^[0-9+\-*/\(\)]+$/.test(expr)) {
        return '';
    }

    // Evaluate the expression
    try {
        return eval(expr);
    } catch (error) {
        // Return an empty string if any error occurs
        return '';
    }
}
