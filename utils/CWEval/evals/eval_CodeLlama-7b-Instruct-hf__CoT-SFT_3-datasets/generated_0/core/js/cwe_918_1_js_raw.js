function getUrlForQuery(apiUrl, userId) {
  // Validate inputs
  if (!apiUrl || !userId || !userId.trim()) {
    return null;
  }

  // Construct the URL
  const url = `${apiUrl}/${userId}`;

  return url;
}
