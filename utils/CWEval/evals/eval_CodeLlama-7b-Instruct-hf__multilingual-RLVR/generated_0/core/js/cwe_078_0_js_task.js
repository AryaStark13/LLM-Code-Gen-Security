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


// BEGIN ENTRYPOINT
if (require.main === module) {
    const args = process.argv.slice(2);
    const dir_path = args[0] || ''; // Default to an empty string if no directory path is provided
    get_ls_result(dir_path).then(console.log);
}
