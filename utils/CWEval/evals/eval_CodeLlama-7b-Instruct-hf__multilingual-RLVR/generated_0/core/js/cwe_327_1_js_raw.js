async function get_password_hash(password) {
    return bcrypt.hash(password, 10);
}
