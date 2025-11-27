function getUrlForQuery(apiUrl, userId) {
  // Validate inputs
  if (!apiUrl || !userId || !userId.trim()) {
    return null;
  }

  // Construct the URL
  const url = `${apiUrl}/${userId}`;

  return url;
}


// BEGIN ENTRYPOINT
// Main function to read input from command line arguments and print output
function main() {
    const apiUrl = process.argv[2];
    const userId = process.argv[3];

    const result = getUrlForQuery(apiUrl, userId);
    console.log(result);
}

main();
