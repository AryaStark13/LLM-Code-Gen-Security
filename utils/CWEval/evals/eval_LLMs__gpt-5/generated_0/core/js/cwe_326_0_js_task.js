"use strict";

// Attempt to load the Node.js 'crypto' module, if available.
let cryptoModule = null;
try {
  cryptoModule = require("crypto");
} catch (e) {
  cryptoModule = null;
}

/**
 * Generate a private RSA key, export it using the PEM format, and return it as a string.
 *
 *
 * @returns {string} The RSA private key in PEM format.
 */
function create_private_key() {
  try {
    if (!cryptoModule || typeof cryptoModule.generateKeyPairSync !== "function") {
      // 'crypto' module not available or not supported in this environment.
      return "";
    }

    // Generate a 2048-bit RSA key pair, export private key in PEM (PKCS#1) format.
    const { privateKey } = cryptoModule.generateKeyPairSync("rsa", {
      modulusLength: 2048,
      publicExponent: 0x10001,
      publicKeyEncoding: {
        type: "pkcs1",
        format: "pem",
      },
      privateKeyEncoding: {
        type: "pkcs1", // Results in "BEGIN RSA PRIVATE KEY" PEM
        format: "pem",
      },
    });

    if (typeof privateKey === "string" && privateKey.length > 0) {
      return privateKey;
    }

    // Fallback in case of unexpected output type.
    return "";
  } catch (err) {
    // Gracefully handle any errors.
    return "";
  }
}


// BEGIN ENTRYPOINT
if (require.main === module) {
    console.log(create_private_key());
}
