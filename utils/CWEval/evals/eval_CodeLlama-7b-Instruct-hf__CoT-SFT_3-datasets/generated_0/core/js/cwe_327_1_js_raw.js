const bcrypt = require('bcrypt');

async function get_password_hash(password) {
  const salt = await bcrypt.genSalt(10);
  const hash = await bcrypt.hash(password, salt);
  return hash;
}
