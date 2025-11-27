function getUrlForQuery(apiUrl, userId) {
  if (!userId || userId.length === 0) {
    return null;
  }
  
  return apiUrl + '/' + userId;
}
