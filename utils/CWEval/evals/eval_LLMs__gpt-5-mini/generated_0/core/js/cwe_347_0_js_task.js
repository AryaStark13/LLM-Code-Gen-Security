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
    // Basic type checks
    if (typeof token !== 'string' || token.length === 0) return {};
    if (typeof key !== 'string') return {};

    // Verify the token using HS256 and return the payload (claims)
    const payload = jwt.verify(token, key, { algorithms: ['HS256'] });

    // Ensure the payload is an object (claims). If not, return empty object.
    if (payload && typeof payload === 'object') {
      return payload;
    }

    return {};
  } catch (err) {
    // On any error (invalid, expired, wrong signature, etc.) return empty object
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
