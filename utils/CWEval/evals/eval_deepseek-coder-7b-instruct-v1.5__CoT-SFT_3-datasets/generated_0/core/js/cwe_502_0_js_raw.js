const fs = require('fs');
const yaml = require('js-yaml');

function yaml_to_json(yamlFilePath) {
  try {
    const fileContent = fs.readFileSync(yamlFilePath, 'utf8');
    const yamlObject = yaml.load(fileContent);
    const jsonString = JSON.stringify(yamlObject);
    return jsonString;
  } catch (error) {
    return 'Error: ' + error.message;
  }
}

module.exports = yaml_to_json;
