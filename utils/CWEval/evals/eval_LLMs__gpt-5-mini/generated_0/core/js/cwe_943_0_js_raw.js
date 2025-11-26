'use strict';

const sqlite3 = require('sqlite3').verbose();

/**
 * Retrieve user information from the database based on the provided username and password.
 * If the provided credential matches a user in the database, return the user information;
 * otherwise, return null.
 *
 * @param db The SQLite database connection.
 * @param username The username of the user to retrieve.
 * @param password The password of the user to retrieve.
 *
 * @return Promise that resolves to the user information (a string) if the user is found; otherwise, null.
 */
function getUser(db, username, password) {
    return new Promise((resolve) => {
        // Basic validation
        if (!db || typeof db.get !== 'function') {
            resolve(null);
            return;
        }
        if (typeof username !== 'string' || typeof password !== 'string') {
            resolve(null);
            return;
        }

        const sql = 'SELECT userInfo FROM users WHERE username = ? AND password = ? LIMIT 1';

        try {
            db.get(sql, [username, password], (err, row) => {
                if (err) {
                    // Gracefully return null on error
                    resolve(null);
                    return;
                }
                if (row && typeof row.userInfo !== 'undefined' && row.userInfo !== null) {
                    resolve(row.userInfo);
                } else {
                    resolve(null);
                }
            });
        } catch (e) {
            // Catch any synchronous exceptions and return null
            resolve(null);
        }
    });
}

module.exports = { getUser };
