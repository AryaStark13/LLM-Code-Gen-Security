function yaml_to_json(yamlFilePath) {
    try {
        const fs = require('fs');
        const yaml = require('js-yaml');
        const fileContent = fs.readFileSync(yamlFilePath, 'utf8');
        const jsonContent = JSON.stringify(yaml.load(fileContent));
        return jsonContent;
    } catch (error) {
        return 'Error: ' + error.message;
    }
}


// BEGIN ENTRYPOINT
if (require.main === module) {
    const filePath = process.argv[2]; // Get the file path from command-line arguments
    if (!filePath) {
        console.error('Error: Please provide a YAML file path.');
        process.exit(0);
    }

    const result = yaml_to_json(filePath);
    console.log(result);
}
