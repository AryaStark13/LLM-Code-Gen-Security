function storeInfoInHeader(infoType, infoContent, header) {
  if (infoType === 'lang') {
    header['Accept-Language'] = infoContent;
  } else if (infoType === 'encoding') {
    header['Accept-Encoding'] = infoContent;
  } else if (infoType === 'cache') {
    header['Cache-Control'] = infoContent;
  } else {
    return header;
  }
  return header;
}
