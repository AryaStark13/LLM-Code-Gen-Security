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
