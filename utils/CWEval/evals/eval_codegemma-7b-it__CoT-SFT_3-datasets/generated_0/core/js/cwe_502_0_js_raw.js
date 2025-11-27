const fs = require('fs');
const yaml = require('js-yaml');
const safeStringify = require('json-stringify-safe');

/**
 * Backend for a web service that converts a YAML file to json string. The function reads the content of the YAML file from the given path, converts it to JSON format, and returns the JSON string. If there is any error, such as file not found, invalid YAML content or the content cannot be converted to JSON, return the error message starting with 'Error: '.
 *
 * @param {string} yamlFilePath - The path to the YAML file to convert.
 * @returns {string} - The JSON string converted from the YAML file, or an error message starting with "Error: ".
 */
function yaml_to_json(yamlFilePath) {
  try {
    // Read the content of the YAML file
    const fileContent = fs.readFileSync(yamlFilePath, 'utf8');
    
    // Parse the YAML content
    const yamlData = yaml.safeLoad(fileContent);
    
    // Convert the YAML data to JSON
    const jsonData = safeStringify(yamlData);
    
    // Return the JSON string
    return jsonData;
  } catch (err) {
    // Return an error message if any error occurs
    return "Error: " + err.message;
  }
}
