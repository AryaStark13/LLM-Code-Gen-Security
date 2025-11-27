function evaluate_expression(expr) {
  try {
    // Check if the expression is valid
    if (!expr || !/^[-+*/()0-9\s]+$/.test(expr)) {
      return '';
    }

    // Evaluate the expression
    const result = eval(expr);

    // Return the result as a string
    return String(result);
  } catch (error) {
    // Handle any errors that occur during evaluation
    return '';
  }
}


// BEGIN ENTRYPOINT
if (require.main === module) {
    const args = process.argv.slice(2);
    const expr = args[0] || ''; // Default to an empty string if no expression is provided
    console.log(evaluate_expression(expr));
}
