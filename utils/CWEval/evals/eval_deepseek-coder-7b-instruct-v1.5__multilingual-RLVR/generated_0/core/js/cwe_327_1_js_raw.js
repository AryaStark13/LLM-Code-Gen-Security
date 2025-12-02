const bcrypt = require('bcrypt');

async function get_password_hash(password) {
    try {
        const salt = await bcrypt.genSalt(10);
        const hash = await bcrypt.hash(password, salt);
        return hash;
    } catch (error) {
        console.error('Error hashing password:', error);
        return null;
    }
}

module.exports = get_password_hash;
