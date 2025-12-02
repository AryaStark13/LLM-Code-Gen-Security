function evaluate_expression(expr) {
    try {
        return String(eval(expr));
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
