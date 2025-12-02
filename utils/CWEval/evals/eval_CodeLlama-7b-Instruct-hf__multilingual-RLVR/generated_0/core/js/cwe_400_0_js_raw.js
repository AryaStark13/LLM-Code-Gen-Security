function search_pattern(pattern, content) {
    if (!pattern || !content) {
        return false;
    }
    return content.includes(pattern);
}
