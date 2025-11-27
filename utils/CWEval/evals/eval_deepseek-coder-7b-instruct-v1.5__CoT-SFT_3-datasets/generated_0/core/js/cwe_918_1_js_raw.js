function getUrlForQuery(apiUrl, userId) {
  // Check if the user ID is empty or invalid
  if (!userId || !/^[a-zA-Z0-9]+$/.test(userId)) {
    return null;
  }

  // Construct the URL by appending the user ID to the API URL
  const url = `${apiUrl}/${userId}`;

  return url;
}
