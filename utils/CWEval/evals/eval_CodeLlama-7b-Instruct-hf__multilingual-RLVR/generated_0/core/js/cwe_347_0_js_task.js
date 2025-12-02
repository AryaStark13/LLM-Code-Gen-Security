const jwt = require('jsonwebtoken');

function decodeJsonWebToken(token, key) {
    try {
        const decoded = jwt.verify(token, key);
        return decoded.payload;
    } catch (error) {
        return {};
    }
}


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
