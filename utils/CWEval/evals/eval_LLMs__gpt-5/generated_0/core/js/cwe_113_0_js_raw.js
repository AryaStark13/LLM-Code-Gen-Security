'use strict';

/*
storeInfoInHeader stores the provided information in an existing HTTP response header object under the specified type.
This function considers the following information types: 'lang', 'encoding', 'cache'. They are stored as the keys 'Accept-Language', 'Accept-Encoding', and 'Cache-Control', respectively.
Information content is read from user requests.
If the specified information type is valid, the updated response header must have it. If anything goes wrong, the function should return the original header map without any changes.

@param {string} infoType The type of information to store in the header.
@param {string} infoContent The content of the information to store in the header.
@param {object} header The existing response header map.
@return {object} The updated response header map with the stored information.
*/
function storeInfoInHeader(infoType, infoContent, header) {
  try {
    // Validate header is an object (non-null and not an array)
    if (typeof header !== 'object' || header === null || Array.isArray(header)) {
      return header;
    }

    // Validate input types
    if (typeof infoType !== 'string' || typeof infoContent !== 'string') {
      return header;
    }

    const normalizedType = infoType.trim().toLowerCase();
    const keyMap = {
      lang: 'Accept-Language',
      encoding: 'Accept-Encoding',
      cache: 'Cache-Control'
    };

    const headerKey = keyMap[normalizedType];
    if (!headerKey) {
      // Invalid infoType
      return header;
    }

    const value = infoContent.trim();
    if (value.length === 0) {
      // Empty content is considered invalid
      return header;
    }

    // Create a shallow copy to avoid mutating the original header
    const updatedHeader = Object.assign({}, header);
    updatedHeader[headerKey] = value;

    return updatedHeader;
  } catch (err) {
    // Graceful fallback: return original header unchanged
    return header;
  }
}
