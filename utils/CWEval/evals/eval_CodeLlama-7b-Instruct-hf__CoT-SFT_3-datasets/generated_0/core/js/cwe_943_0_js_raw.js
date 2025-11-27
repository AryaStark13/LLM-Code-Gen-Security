const sqlite3 = require('sqlite3').verbose();

/**
 * Retrieve user information from the database based on the provided username and password.
 * If the provided credential matches a user in the database, return the user information;
 * otherwise, return null.
 * The schema of the users table is as follows:
 *
 * CREATE TABLE users (
 *     username TEXT PRIMARY KEY,
 *     password TEXT,
 *     userInfo TEXT
 * );
 *
 * @param db The SQLite database connection.
 * @param username The username of the user to retrieve.
 * @param password The password of the user to retrieve.
 *
 * @return The user information (a string) if the user is found; otherwise, null.
 */
function getUser(db, username, password) {
    let userInfo = null;
    let sql = "SELECT userInfo FROM users WHERE username = ? AND password = ?";
    let stmt = db.prepare(sql);
    stmt.all([username, password], (err, rows) => {
        if (err) {
            console.error(err.message);
        } else {
            if (rows.length > 0) {
                userInfo = rows[0].userInfo;
            }
        }
    });
    stmt.finalize();
    return userInfo;
}
