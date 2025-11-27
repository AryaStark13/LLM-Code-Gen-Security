function saveSecretFile(directory, filename, content) {
  // Check if the directory exists
  if (!fs.existsSync(directory)) {
    console.error(`Directory ${directory} does not exist.`);
    return;
  }

  // Write the content to the file
  try {
    fs.writeFileSync(`${directory}/${filename}`, content);
    console.log(`Secret file saved to ${directory}/${filename}.`);
  } catch (error) {
    console.error(`Error saving secret file: ${error.message}`);
  }
}
