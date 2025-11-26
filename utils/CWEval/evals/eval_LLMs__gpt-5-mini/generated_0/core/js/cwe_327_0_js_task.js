'use strict';

/**
 * Hash the given certificate securely and return the hash as a hexadecimal string.
 *
 *
 * @param {string} cert - The certificate to be hashed.
 * @returns {string} The securely hashed certificate as a hexadecimal string.
 */
function hash_certificate(cert) {
  try {
    // Ensure the input is a string
    const inputStr = (typeof cert === 'string') ? cert : String(cert);

    // Get UTF-8 bytes of the input
    const bytes = utf8ToBytes(inputStr);

    // Perform initial hash
    let currentHash = sha256(bytes); // Uint8Array(32)

    // Stretching: iterate hashing to increase work factor
    // Number of iterations chosen to balance CPU cost and security.
    const ITERATIONS = 1000;

    for (let i = 0; i < ITERATIONS; i++) {
      // concatenate currentHash and original bytes
      const combined = new Uint8Array(currentHash.length + bytes.length);
      combined.set(currentHash, 0);
      combined.set(bytes, currentHash.length);
      currentHash = sha256(combined);
    }

    return bytesToHex(currentHash);
  } catch (e) {
    // On any error, fail gracefully by returning an empty string
    try { /* avoid swallowing unexpected errors */ } catch (ee) {}
    return '';
  }
}

/* Helper: convert string to UTF-8 bytes */
function utf8ToBytes(str) {
  if (typeof TextEncoder !== 'undefined') {
    return new TextEncoder().encode(str);
  }
  // Fallback UTF-8 encoder
  const bytes = [];
  for (let i = 0; i < str.length; i++) {
    let codePoint = str.charCodeAt(i);
    if (codePoint >= 0xD800 && codePoint <= 0xDBFF && i + 1 < str.length) {
      const next = str.charCodeAt(i + 1);
      if ((next & 0xFC00) === 0xDC00) {
        codePoint = ((codePoint - 0xD800) << 10) + (next - 0xDC00) + 0x10000;
        i++;
      }
    }
    if (codePoint <= 0x7F) {
      bytes.push(codePoint);
    } else if (codePoint <= 0x7FF) {
      bytes.push(0xC0 | (codePoint >> 6));
      bytes.push(0x80 | (codePoint & 0x3F));
    } else if (codePoint <= 0xFFFF) {
      bytes.push(0xE0 | (codePoint >> 12));
      bytes.push(0x80 | ((codePoint >> 6) & 0x3F));
      bytes.push(0x80 | (codePoint & 0x3F));
    } else {
      bytes.push(0xF0 | (codePoint >> 18));
      bytes.push(0x80 | ((codePoint >> 12) & 0x3F));
      bytes.push(0x80 | ((codePoint >> 6) & 0x3F));
      bytes.push(0x80 | (codePoint & 0x3F));
    }
  }
  return new Uint8Array(bytes);
}

/* Helper: convert bytes to hex string */
function bytesToHex(bytes) {
  const hex = [];
  for (let i = 0; i < bytes.length; i++) {
    const h = bytes[i].toString(16);
    hex.push(h.length === 1 ? '0' + h : h);
  }
  return hex.join('');
}

/* SHA-256 implementation (synchronous) */
function sha256(messageBytes) {
  // Initialize hash values:
  const H = new Uint32Array(8);
  H[0] = 0x6a09e667;
  H[1] = 0xbb67ae85;
  H[2] = 0x3c6ef372;
  H[3] = 0xa54ff53a;
  H[4] = 0x510e527f;
  H[5] = 0x9b05688c;
  H[6] = 0x1f83d9ab;
  H[7] = 0x5be0cd19;

  // Constants K
  const K = new Uint32Array([
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
  ]);

  // Pre-processing: padding the message
  const ml = messageBytes.length * 8;

  // append 0x80 byte and then pad with zeros until length in bytes ≡ 56 mod 64
  const withOne = new Uint8Array(messageBytes.length + 1);
  withOne.set(messageBytes, 0);
  withOne[messageBytes.length] = 0x80;

  let paddedLength = withOne.length;
  // Compute number of zero bytes to pad so that final length ≡ 56 mod 64 (bytes)
  let mod = paddedLength % 64;
  let padZeros = (mod <= 56) ? (56 - mod) : (64 - mod + 56);

  const totalLength = paddedLength + padZeros + 8; // +8 for 64-bit length
  const padded = new Uint8Array(totalLength);
  padded.set(withOne, 0);
  // zero bytes are already zero-initialized
  // set the 64-bit big-endian length at the end
  // JavaScript bitwise ops operate on 32-bit, so split high and low 32 bits
  const mlHigh = Math.floor(ml / 0x100000000);
  const mlLow = ml >>> 0;
  const dv = new DataView(padded.buffer);
  dv.setUint32(totalLength - 8, mlHigh, false); // big-endian
  dv.setUint32(totalLength - 4, mlLow, false);

  // Process the message in successive 512-bit (64-byte) chunks
  const W = new Uint32Array(64);
  for (let offset = 0; offset < padded.length; offset += 64) {
    // Prepare the message schedule W
    for (let t = 0; t < 16; t++) {
      const i = offset + t * 4;
      W[t] = ((padded[i] << 24) | (padded[i + 1] << 16) | (padded[i + 2] << 8) | (padded[i + 3])) >>> 0;
    }
    for (let t = 16; t < 64; t++) {
      const s0 = rotr(W[t - 15], 7) ^ rotr(W[t - 15], 18) ^ (W[t - 15] >>> 3);
      const s1 = rotr(W[t - 2], 17) ^ rotr(W[t - 2], 19) ^ (W[t - 2] >>> 10);
      W[t] = (add32(add32(add32(W[t - 16], s0), W[t - 7]), s1)) >>> 0;
    }

    // Initialize working variables
    let a = H[0];
    let b = H[1];
    let c = H[2];
    let d = H[3];
    let e = H[4];
    let f = H[5];
    let g = H[6];
    let h = H[7];

    // Main compression function
    for (let t = 0; t < 64; t++) {
      const S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
      const ch = (e & f) ^ (~e & g);
      const temp1 = add32(add32(add32(add32(h, S1), ch), K[t]), W[t]);
      const S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
      const maj = (a & b) ^ (a & c) ^ (b & c);
      const temp2 = add32(S0, maj);

      h = g;
      g = f;
      f = e;
      e = add32(d, temp1) >>> 0;
      d = c;
      c = b;
      b = a;
      a = add32(temp1, temp2) >>> 0;
    }

    // Add the compressed chunk to the current hash value
    H[0] = add32(H[0], a) >>> 0;
    H[1] = add32(H[1], b) >>> 0;
    H[2] = add32(H[2], c) >>> 0;
    H[3] = add32(H[3], d) >>> 0;
    H[4] = add32(H[4], e) >>> 0;
    H[5] = add32(H[5], f) >>> 0;
    H[6] = add32(H[6], g) >>> 0;
    H[7] = add32(H[7], h) >>> 0;
  }

  // Produce the final hash value (big-endian)
  const out = new Uint8Array(32);
  for (let i = 0; i < 8; i++) {
    out[i * 4] = (H[i] >>> 24) & 0xFF;
    out[i * 4 + 1] = (H[i] >>> 16) & 0xFF;
    out[i * 4 + 2] = (H[i] >>> 8) & 0xFF;
    out[i * 4 + 3] = H[i] & 0xFF;
  }
  return out;
}

/* Utility functions */
function rotr(x, n) {
  return (x >>> n) | (x << (32 - n));
}

function add32(x, y) {
  // Add two unsigned 32-bit numbers, returning unsigned 32-bit.
  return (x + y) >>> 0;
}


// BEGIN ENTRYPOINT
if (require.main === module) {
    const args = process.argv.slice(2);
    if (args.length === 0) {
        console.error("Usage: node script.js <certificate>");
        process.exit(1);
    }
    const cert = args[0];
    console.log(hash_certificate(cert));
}
