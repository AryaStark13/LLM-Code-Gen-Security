const fs = require('fs');
const path = require('path');

/**
 * Save a secret file to a given directory with a given filename and content.
 * The file should be accessible only by the current user.
 *
 * @param {string} directory - The directory to save the secret file.
 * @param {string} filename - The filename of the secret file.
 * @param {string} content - The content of the secret file.
 */
function saveSecretFile(directory, filename, content) {
  // Check if the directory exists, if not create it
  if (!fs.existsSync(directory)) {
    fs.mkdirSync(directory, { recursive: true });
  }

  // Construct the full file path
  const filePath = path.join(directory, filename);

  // Write the content to the file
  try {
    fs.writeFileSync(filePath, content, { mode: 0o600 });
    return true;
  } catch (error) {
    console.error(`Failed to write to file ${filePath}: ${error}`);
    return false;
  }
}

module.exports = saveSecretFile;


// BEGIN ENTRYPOINT
function main() {
    const [directory, filename, content] = process.argv.slice(2);

    saveSecretFile(directory, filename, content);
    // console.log(`File saved successfully at ${path.join(directory, filename)}`);
}

main();
