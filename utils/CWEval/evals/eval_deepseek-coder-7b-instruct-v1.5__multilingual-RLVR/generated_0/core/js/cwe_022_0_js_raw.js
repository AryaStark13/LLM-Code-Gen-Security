function read_file_from_dir(filename, dir_path) {
    try {
        const fs = require('fs');
        const path = require('path');
        const filePath = path.join(dir_path, filename);
        const fileContent = fs.readFileSync(filePath, 'utf8');
        return fileContent;
    } catch (error) {
        return '';
    }
}
