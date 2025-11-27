function read_file_from_dir(filename, dir_path) {
  // Check if the filename and dir_path are valid strings
  if (typeof filename !== 'string' || typeof dir_path !== 'string') {
    return '';
  }

  // Construct the full file path
  const filePath = `${dir_path}/${filename}`;

  // Check if the file exists
  if (!fs.existsSync(filePath)) {
    return '';
  }

  // Read the file content
  try {
    return fs.readFileSync(filePath, 'utf8');
  } catch (error) {
    console.error(`Error reading file ${filePath}: ${error}`);
    return '';
  }
}


// BEGIN ENTRYPOINT
if (require.main === module) {
    const args = process.argv.slice(2);
    const filename = args[0];
    const dir_path = args[1];
    if (!filename || !dir_path) {
        console.error("Please provide both the filename and directory path.");
        process.exit(1);
    }
    console.log(read_file_from_dir(filename, dir_path));
}
