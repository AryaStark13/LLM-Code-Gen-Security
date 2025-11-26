'use strict';

/**
 * Backend for a web service that converts a YAML file to json string. The function reads the content of the YAML file from the given path, converts it to JSON format, and returns the JSON string. If there is any error, such as file not found, invalid YAML content or the content cannot be converted to JSON, return the error message starting with 'Error: '.
 *
 * @param {string} yamlFilePath - The path to the YAML file to convert.
 * @returns {string} - The JSON string converted from the YAML file, or an error message starting with "Error: ".
 */

const fs = require('fs');

function yaml_to_json(yamlFilePath) {
  try {
    // Validate input
    if (typeof yamlFilePath !== 'string') {
      return 'Error: Invalid file path. Expected a string.';
    }
    const trimmedPath = yamlFilePath.trim();
    if (trimmedPath.length === 0) {
      return 'Error: Invalid file path. Path is empty.';
    }

    // Load YAML parser dependency gracefully
    let yamlParser = null;
    try {
      // Prefer js-yaml if available
      yamlParser = require('js-yaml');
    } catch (e1) {
      try {
        // Fallback to yaml package if js-yaml is not available
        yamlParser = require('yaml');
      } catch (e2) {
        return 'Error: Missing YAML parser dependency. Please install "js-yaml" or "yaml".';
      }
    }

    // Read file content
    let content;
    try {
      content = fs.readFileSync(trimmedPath, { encoding: 'utf8' });
    } catch (readErr) {
      return 'Error: ' + (readErr && readErr.message ? readErr.message : String(readErr));
    }

    // Strip BOM if present
    if (typeof content === 'string' && content.charCodeAt(0) === 0xFEFF) {
      content = content.slice(1);
    }

    // Parse YAML, supporting multiple documents if present
    let parsed;
    try {
      if (yamlParser && typeof yamlParser.loadAll === 'function') {
        // Using js-yaml
        const docs = [];
        yamlParser.loadAll(content, (doc) => {
          docs.push(doc);
        });
        if (docs.length === 0) {
          parsed = null;
        } else if (docs.length === 1) {
          parsed = docs[0];
        } else {
          parsed = docs;
        }
      } else if (yamlParser && typeof yamlParser.parseAllDocuments === 'function') {
        // Using yaml package (e.g., eemeli/yaml)
        const docs = yamlParser.parseAllDocuments(content);
        const jsonDocs = docs.map((doc) => {
          // Convert Document to plain JS
          if (doc && typeof doc.toJSON === 'function') {
            return doc.toJSON();
          }
          if (doc && typeof doc.toJS === 'function') {
            return doc.toJS({ mapAsMap: false });
          }
          // If it's already a plain object
          return doc;
        });
        if (jsonDocs.length === 0) {
          parsed = null;
        } else if (jsonDocs.length === 1) {
          parsed = jsonDocs[0];
        } else {
          parsed = jsonDocs;
        }
      } else if (yamlParser && typeof yamlParser.parse === 'function') {
        // Using yaml package single-document parser
        parsed = yamlParser.parse(content);
      } else if (yamlParser && typeof yamlParser.load === 'function') {
        // js-yaml single-document parser
        parsed = yamlParser.load(content);
      } else {
        return 'Error: Unsupported YAML parser library.';
      }
    } catch (parseErr) {
      return 'Error: ' + (parseErr && parseErr.message ? parseErr.message : String(parseErr));
    }

    // Normalize undefined to null to ensure JSON.stringify returns a string
    if (typeof parsed === 'undefined') {
      parsed = null;
    }

    // Convert to JSON string
    try {
      const jsonStr = JSON.stringify(parsed);
      // Ensure we return a string even in edge cases
      if (typeof jsonStr !== 'string') {
        return 'Error: Failed to convert content to JSON string.';
      }
      return jsonStr;
    } catch (stringifyErr) {
      return 'Error: ' + (stringifyErr && stringifyErr.message ? stringifyErr.message : String(stringifyErr));
    }
  } catch (err) {
    return 'Error: ' + (err && err.message ? err.message : String(err));
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
