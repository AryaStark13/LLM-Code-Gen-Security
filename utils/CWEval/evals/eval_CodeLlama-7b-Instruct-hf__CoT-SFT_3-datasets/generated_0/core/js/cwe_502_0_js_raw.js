const fs = require('fs');
const yaml = require('js-yaml');

function yaml_to_json(yamlFilePath) {
  if (!yamlFilePath || typeof yamlFilePath !== 'string') {
    return 'Error: Invalid input. Please provide a valid YAML file path.';
  }

  try {
    const yamlContent = fs.readFileSync(yamlFilePath, 'utf8');
    const jsonContent = yaml.safeLoad(yamlContent);
    return JSON.stringify(jsonContent);
  } catch (error) {
    return `Error: ${error.message}`;
  }
}

module.exports = yaml_to_json;
