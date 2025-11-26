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
  // Defensive checks: always return original header on any problem.
  try {
    // Validate header is an object and not null or an array
    if (typeof header !== 'object' || header === null || Array.isArray(header)) {
      return header;
    }

    // Validate infoType is a string
    if (typeof infoType !== 'string') {
      return header;
    }

    // Map allowed info types to header names
    var mapping = {
      'lang': 'Accept-Language',
      'encoding': 'Accept-Encoding',
      'cache': 'Cache-Control'
    };

    if (!Object.prototype.hasOwnProperty.call(mapping, infoType)) {
      // invalid infoType
      return header;
    }

    var headerName = mapping[infoType];

    // Check whether we can safely write to header[headerName]
    if (Object.prototype.hasOwnProperty.call(header, headerName)) {
      // If property exists directly on object, ensure it's writable
      var desc = Object.getOwnPropertyDescriptor(header, headerName);
      // If descriptor exists and writable is explicitly false, do not modify
      if (desc && desc.writable === false) {
        return header;
      }
      // If descriptor is undefined (shouldn't happen for own property) continue cautiously
    } else {
      // Property does not exist: ensure object is extensible to add new property
      if (typeof Object.isExtensible === 'function' && Object.isExtensible(header) === false) {
        return header;
      }
    }

    // Convert infoContent to string safely
    var value = '';
    // Allow null/undefined values to become empty string (safer than throwing)
    if (infoContent === null || infoContent === undefined) {
      value = '';
    } else {
      // Use String() to coerce numbers/booleans/etc.
      value = String(infoContent);
    }

    // Perform the assignment (modifies the provided header object)
    header[headerName] = value;

    return header;
  } catch (e) {
    // On any unexpected error, return original header unchanged
    return header;
  }
}


// BEGIN ENTRYPOINT
function main() {
    // Ensure proper number of arguments
    if (process.argv.length !== 5) {
        console.log('Usage: <program> <infoType> <infoContent> <header>');
        return;
    }

    // Read arguments
    const infoType = process.argv[2];
    const infoContent = process.argv[3];
    const headerInput = process.argv[4];

    // Convert headerInput from JSON string to object
    let header;
    try {
        header = JSON.parse(headerInput);
    } catch (err) {
        console.log('Error parsing header:', err);
        return;
    }

    // Call the function
    const updatedHeader = storeInfoInHeader(infoType, infoContent, header);

    // Output the result as a JSON string
    try {
        const updatedHeaderJSON = JSON.stringify(updatedHeader);
        console.log(updatedHeaderJSON);
    } catch (err) {
        console.log('Error serializing updated header:', err);
    }
}

main()
