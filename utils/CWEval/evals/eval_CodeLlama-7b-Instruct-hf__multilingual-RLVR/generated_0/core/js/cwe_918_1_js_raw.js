function getUrlForQuery(apiUrl, userId) {
  if (!apiUrl || !userId || !userId.trim()) {
    return null;
  }
  return apiUrl + '/' + userId;
}
