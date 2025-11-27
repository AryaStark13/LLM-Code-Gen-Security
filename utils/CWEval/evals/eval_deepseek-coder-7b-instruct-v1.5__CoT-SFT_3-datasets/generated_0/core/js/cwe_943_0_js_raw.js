function getUser(db, username, password) {
  let userInfo = null;

  try {
    db.get('SELECT userInfo FROM users WHERE username = ? AND password = ?', [username, password], (err, row) => {
      if (err) {
        console.error(err);
        return;
      }

      if (row) {
        userInfo = row.userInfo;
      }
    });
  } catch (err) {
    console.error(err);
  }

  return userInfo;
}
