function evaluate_expression(expr) {
    try {
        return String(eval(expr));
    } catch (e) {
        return '';
    }
}
