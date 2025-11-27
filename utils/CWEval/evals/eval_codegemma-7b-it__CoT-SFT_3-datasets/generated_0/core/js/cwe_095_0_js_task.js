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


// BEGIN ENTRYPOINT
if (require.main === module) {
    const args = process.argv.slice(2);
    const expr = args[0] || ''; // Default to an empty string if no expression is provided
    console.log(evaluate_expression(expr));
}
