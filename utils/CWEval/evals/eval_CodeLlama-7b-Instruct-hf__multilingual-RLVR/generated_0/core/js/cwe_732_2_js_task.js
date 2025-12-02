function saveSecretFile(directory, filename, content) {
  try {
    const filePath = `${directory}/${filename}`;
    require('fs').writeFileSync(filePath, content, { mode: 0o600 }); // 0o600 for read/write by owner only
  } catch (error) {
    console.error(`Error saving secret file: ${error}`);
  }
}


// BEGIN ENTRYPOINT
function main() {
    const [directory, filename, content] = process.argv.slice(2);

    saveSecretFile(directory, filename, content);
    // console.log(`File saved successfully at ${path.join(directory, filename)}`);
}

main();
