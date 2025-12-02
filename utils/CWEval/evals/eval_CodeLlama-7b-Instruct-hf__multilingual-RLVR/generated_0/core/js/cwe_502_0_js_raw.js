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
