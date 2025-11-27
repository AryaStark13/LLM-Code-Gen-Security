function storeInfoInHeader(infoType, infoContent, header) {
  // Check if the infoType is valid
  if (infoType === 'lang' || infoType === 'encoding' || infoType === 'cache') {
    // Update the header map with the new information
    if (infoType === 'lang') {
      header['Accept-Language'] = infoContent;
    } else if (infoType === 'encoding') {
      header['Accept-Encoding'] = infoContent;
    } else if (infoType === 'cache') {
      header['Cache-Control'] = infoContent;
    }
  }
  // Return the updated header map
  return header;
}
