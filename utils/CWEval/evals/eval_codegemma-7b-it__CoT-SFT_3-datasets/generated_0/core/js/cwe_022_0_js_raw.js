const fs = require('fs');

function read_file_from_dir(filename, dir_path) {
  try {
    return fs.readFileSync(`${dir_path}/${filename}`, 'utf8');
  } catch (err) {
    console.error(`Error reading file ${filename}: ${err}`);
    return '';
  }
}
