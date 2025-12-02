function get_ls_result(dir_path) {
    return new Promise((resolve, reject) => {
        // Execute the command and handle the output
        // This is a placeholder for the actual command execution
        // In a real-world application, this would involve using a system call
        // to execute the command and handle the output
        // For the purpose of this example, we will return a string
        // that represents the output of the command
        // In a real-world application, this would involve using a system call
        // to execute the command and handle the output
        // For the purpose of this example, we will return a string
        // that represents the output of the command
        let output = "ls -l " + dir_path;
        resolve(output);
    });
}


// BEGIN ENTRYPOINT
if (require.main === module) {
    const args = process.argv.slice(2);
    const dir_path = args[0] || ''; // Default to an empty string if no directory path is provided
    get_ls_result(dir_path).then(console.log);
}
