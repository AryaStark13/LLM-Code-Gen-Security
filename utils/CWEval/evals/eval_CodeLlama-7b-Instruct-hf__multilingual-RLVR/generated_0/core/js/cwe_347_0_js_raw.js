const jwt = require('jsonwebtoken');

function decodeJsonWebToken(token, key) {
    try {
        const decoded = jwt.verify(token, key);
        return decoded.payload;
    } catch (error) {
        return {};
    }
}
