function get_ls_result(dir_path) {
    return new Promise((resolve, reject) => {
        const lsCommand = `ls -l ${dir_path}`;
        require('child_process').exec(lsCommand, (error, stdout, stderr) => {
            if (error) {
                reject(error);
            } else {
                resolve(stdout);
            }
        });
    });
}
