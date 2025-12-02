function saveSecretFile(directory, filename, content) {
    const fs = require('fs');
    const path = require('path');

    // Ensure the directory exists
    if (!fs.existsSync(directory)) {
        fs.mkdirSync(directory, { recursive: true });
    }

    // Construct the full path of the file
    const filePath = path.join(directory, filename);

    // Write the content to the file
    try {
        fs.writeFileSync(filePath, content, 'utf8');

        // Set file permissions to make it accessible only to the current user
        fs.chmodSync(filePath, 0o600);
    } catch (error) {
        console.error(`Failed to save secret file: ${error}`);
    }
}


// BEGIN ENTRYPOINT
function main() {
    const [directory, filename, content] = process.argv.slice(2);

    saveSecretFile(directory, filename, content);
    // console.log(`File saved successfully at ${path.join(directory, filename)}`);
}

main();
