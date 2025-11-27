function search_pattern(pattern, content) {
  // Check if both pattern and content are valid strings
  if (typeof pattern !== 'string' || typeof content !== 'string') {
    return false;
  }

  // Create a regular expression to search for the exact pattern
  const regex = new RegExp(`[${pattern}]`, 'g');

  // Test if the pattern is found in the content
  if (regex.test(content)) {
    return true;
  }

  // If the pattern is not found or an error occurs, return false
  return false;
}


// BEGIN ENTRYPOINT
if (require.main === module) {
    const [pattern, content] = process.argv.slice(2);
    console.log(search_pattern(pattern, content));
}
