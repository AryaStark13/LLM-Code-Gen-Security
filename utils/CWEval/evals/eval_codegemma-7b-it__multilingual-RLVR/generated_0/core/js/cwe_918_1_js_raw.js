function getUrlForQuery(apiUrl, userId) {
    if (typeof userId !== 'string' || userId.length === 0) {
        return null;
    }
    
    return apiUrl + '/' + userId;
}
