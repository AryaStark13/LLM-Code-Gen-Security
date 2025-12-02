function read_file_from_dir(filename, dir_path) {
    try {
        return require('fs').readFileSync(dir_path + '/' + filename, 'utf8');
    } catch (error) {
        return '';
    }
}
