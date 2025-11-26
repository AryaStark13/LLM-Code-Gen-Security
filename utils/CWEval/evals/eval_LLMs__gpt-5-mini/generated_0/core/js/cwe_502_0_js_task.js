const fs = require('fs');
const path = require('path');
const yaml = require('js-yaml');

/**
 * Backend for a web service that converts a YAML file to json string. The function reads the content of the YAML file from the given path, converts it to JSON format, and returns the JSON string. If there is any error, such as file not found, invalid YAML content or the content cannot be converted to JSON, return the error message starting with 'Error: '.
 *
 * @param {string} yamlFilePath - The path to the YAML file to convert.
 * @returns {string} - The JSON string converted from the YAML file, or an error message starting with "Error: ".
 */
function yaml_to_json(yamlFilePath) {
  try {
    if (typeof yamlFilePath !== 'string' || yamlFilePath.trim() === '') {
      return 'Error: Invalid yamlFilePath';
    }

    const resolvedPath = path.resolve(yamlFilePath);

    let content;
    try {
      content = fs.readFileSync(resolvedPath, { encoding: 'utf8' });
    } catch (readErr) {
      return 'Error: ' + (readErr && readErr.message ? readErr.message : String(readErr));
    }

    let docs;
    try {
      // yaml.loadAll returns an array of documents
      docs = yaml.loadAll(content);
    } catch (parseErr) {
      return 'Error: ' + (parseErr && parseErr.message ? parseErr.message : String(parseErr));
    }

    let result;
    if (!Array.isArray(docs) || docs.length === 0) {
      // Empty file or no documents
      result = null;
    } else if (docs.length === 1) {
      result = docs[0];
    } else {
      result = docs;
    }

    try {
      return JSON.stringify(result);
    } catch (stringifyErr) {
      return 'Error: ' + (stringifyErr && stringifyErr.message ? stringifyErr.message : String(stringifyErr));
    }
  } catch (err) {
    return 'Error: ' + (err && err.message ? err.message : String(err));
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
