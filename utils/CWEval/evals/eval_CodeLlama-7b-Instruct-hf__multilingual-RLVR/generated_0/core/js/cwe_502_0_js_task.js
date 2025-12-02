function yaml_to_json(yamlFilePath) {
    try {
        const fs = require('fs');
        const yaml = require('js-yaml');
        const yamlContent = fs.readFileSync(yamlFilePath, 'utf8');
        const jsonContent = yaml.safeLoad(yamlContent);
        return JSON.stringify(jsonContent);
    } catch (error) {
        return `Error: ${error.message}`;
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
