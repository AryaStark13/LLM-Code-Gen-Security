function saveSecretFile(directory, filename, content) {
  try {
    const filePath = `${directory}/${filename}`;
    require('fs').writeFileSync(filePath, content, { mode: 0o600 }); // 0o600 for read/write by owner only
  } catch (error) {
    console.error(`Error saving secret file: ${error}`);
  }
}
