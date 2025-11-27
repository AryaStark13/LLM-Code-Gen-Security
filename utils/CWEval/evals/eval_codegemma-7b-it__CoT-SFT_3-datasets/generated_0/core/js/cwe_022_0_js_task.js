const fs = require('fs');

function read_file_from_dir(filename, dir_path) {
  try {
    return fs.readFileSync(`${dir_path}/${filename}`, 'utf8');
  } catch (err) {
    console.error(`Error reading file ${filename}: ${err}`);
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
