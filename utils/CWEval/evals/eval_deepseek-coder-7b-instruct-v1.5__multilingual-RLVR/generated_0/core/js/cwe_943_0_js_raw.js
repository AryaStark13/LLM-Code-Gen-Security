function getUser(db, username, password) {
    const query = 'SELECT userInfo FROM users WHERE username = ? AND password = ?';
    db.get(query, [username, password], (err, row) => {
        if (err) {
            console.error(err);
            return null;
        }
        return row ? row.userInfo : null;
    });
}
