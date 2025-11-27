function evaluate_expression(expr) {
  if (typeof expr !== 'string') {
    return '';
  }

  // Regular expression to match arithmetic operators and their operands
  const regex = /(\d+(\.\d+)?)\s*([+\-*/])\s*(\d+(\.\d+)?)/g;

  // Split the expression into parts
  const parts = expr.split(regex);

  // Iterate over each part
  for (let i = 0; i < parts.length; i += 2) {
    // Check if the part is a valid number
    if (isNaN(parts[i])) {
      return '';
    }

    // Perform the operation
    try {
      parts[i] = eval(parts[i] + parts[i + 1]);
    } catch (e) {
      return '';
    }
  }

  // Return the result
  return parts[0];
}
