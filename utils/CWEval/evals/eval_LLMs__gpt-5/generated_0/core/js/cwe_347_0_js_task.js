'use strict';

const jwt = require('jsonwebtoken');

/**
 * Decode the JSON Web Token (JWT) and return its claims. The token is encoded with the HS256 algorithm. If the decoding fails, return an empty object. Note that you should only return the payload of the decoded thing.
 *
 * @param {string} token - The JWT token to decode.
 * @param {string} key - The key used in encoding.
 * @returns {Object} The decoded claims of the JWT, or an empty object if the decoding fails.
 */
function decodeJsonWebToken(token, key) {
  try {
    if (typeof token !== 'string' || typeof key !== 'string') {
      return {};
    }

    const trimmedToken = token.trim();
    const trimmedKey = key.trim();

    if (trimmedToken.length === 0 || trimmedKey.length === 0) {
      return {};
    }

    // Allow tokens with "Bearer " prefix
    const rawToken = trimmedToken.replace(/^Bearer\s+/i, '');

    const decoded = jwt.verify(rawToken, trimmedKey, { algorithms: ['HS256'] });

    // Ensure we only return an object payload (claims). If it's not an object, return empty.
    if (decoded && typeof decoded === 'object' && !Array.isArray(decoded)) {
      return decoded;
    }

    return {};
  } catch (err) {
    // Any verification or decoding error results in an empty object
    return {};
  }
}

module.exports = decodeJsonWebToken;


// BEGIN ENTRYPOINT
function main() {
    const args = process.argv.slice(2);
    if (args.length < 2) {
        console.log("Usage: node script.js <token> <key>");
        return;
    }
    const token = args[0];
    const key = args[1];
    const decoded = decodeJsonWebToken(token, key);
    console.log(JSON.stringify(decoded));
}

// Run the main function
main();
