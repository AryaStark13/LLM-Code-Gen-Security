function search_pattern(pattern, content) {
  // Check if the pattern is a string
  if (typeof pattern !== 'string') {
    return false;
  }

  // Check if the content is a string or an array of strings
  if (typeof content !== 'string' && !Array.isArray(content)) {
    return false;
  }

  // If the pattern is an empty string, return false
  if (pattern === '') {
    return false;
  }

  // If the content is an array, check if the pattern is in any of the elements
  if (Array.isArray(content)) {
    for (let i = 0; i < content.length; i++) {
      if (content[i].includes(pattern)) {
        return true;
      }
    }
  }

  // If the content is a string, check if the pattern is a substring of the content
  if (typeof content === 'string') {
    if (content.includes(pattern)) {
      return true;
    }
  }

  // If the pattern is not found in the content, return false
  return false;
}


// BEGIN ENTRYPOINT
if (require.main === module) {
    const [pattern, content] = process.argv.slice(2);
    console.log(search_pattern(pattern, content));
}
