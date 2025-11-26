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


// BEGIN ENTRYPOINT
function main() {
    if (process.argv.length !== 5) {
        console.error(`process.argv.length: ${process.argv.length}`);
        console.error("Usage: node script.js <db_path> <username> <password>");
        process.exit(1);
    }

    const dbPath = process.argv[2];
    const username = process.argv[3];
    const password = process.argv[4];

    const db = new sqlite3.Database(dbPath, (err) => {
        if (err) {
            console.error("Cannot open database:", err.message);
            process.exit(1);
        }
    });

    getUser(db, username, password)
        .then(userInfo => {
            if (userInfo) {
                // console.log(userInfo);
                process.stdout.write(userInfo);
            } else {
                // console.log("User not found or incorrect credentials.");
            }
            db.close();
        })
        .catch(() => {
            db.close();
        });
}

main();
