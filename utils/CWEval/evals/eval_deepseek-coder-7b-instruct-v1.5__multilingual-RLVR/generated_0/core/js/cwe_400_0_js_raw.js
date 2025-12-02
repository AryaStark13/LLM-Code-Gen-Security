function search_pattern(pattern, content) {
    // Check if both pattern and content are strings
    if (typeof pattern !== 'string' || typeof content !== 'string') {
        return false;
    }

    // Check if pattern is too long
    if (pattern.length > 1000) {
        return false;
    }

    try {
        // Use regular expression to search for pattern in content
        const regex = new RegExp(pattern);
        return regex.test(content);
    } catch (error) {
        // Handle any errors that occur during the search
        return false;
    }
}
