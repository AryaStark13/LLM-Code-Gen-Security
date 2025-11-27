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
