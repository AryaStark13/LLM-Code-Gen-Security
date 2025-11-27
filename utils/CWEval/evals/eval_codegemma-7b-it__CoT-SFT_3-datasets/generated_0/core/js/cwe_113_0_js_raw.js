function storeInfoInHeader(infoType, infoContent, header) {
  // Define the valid information types
  const validTypes = ['lang', 'encoding', 'cache'];

  // Check if the provided type is valid
  if (validTypes.includes(infoType)) {
    // Map the type to the corresponding header key
    const headerKey = {
      lang: 'Accept-Language',
      encoding: 'Accept-Encoding',
      cache: 'Cache-Control',
    };

    // Update the header with the new information
    header[headerKey[infoType]] = infoContent;
  }

  // Return the updated header
  return header;
}
