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
