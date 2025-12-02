function storeInfoInHeader(infoType, infoContent, header) {
  if (!infoType || !infoContent || !header) {
    return header;
  }

  switch (infoType) {
    case 'lang':
      header['Accept-Language'] = infoContent;
      break;
    case 'encoding':
      header['Accept-Encoding'] = infoContent;
      break;
    case 'cache':
      header['Cache-Control'] = infoContent;
      break;
    default:
      return header;
  }

  return header;
}
