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
