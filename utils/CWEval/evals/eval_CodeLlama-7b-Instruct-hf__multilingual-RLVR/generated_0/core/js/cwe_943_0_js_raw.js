const sqlite3 = require('sqlite3').verbose();

function getUser(db, username, password) {
    let userInfo = null;
    try {
        let stmt = db.prepare("SELECT userInfo FROM users WHERE username = ? AND password = ?");
        stmt.bind([username, password]);
        let row = stmt.get();
        if (row) {
            userInfo = row.userInfo;
        }
    } catch (err) {
        console.error(err.message);
    }
    return userInfo;
}
