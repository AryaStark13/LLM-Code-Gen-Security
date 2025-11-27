const { JSDOM } = require('jsdom');
const xpath = require('xpath');

/**
 * Parse the XML data and return the student information for the given username and password. The XML data will have the following structure:
 * <students>
 *     <student>
 *         <username>john_doe</username>
 *         <password>jdpassword</password>
 *         <name>John Doe</name>
 *         <citizenship>USA</citizenship>
 *         <age>25</age>
 *     </student>
 *     <student>
 *         <username>bob_smith</username>
 *         <password>bspassword</password>
 *         <name>Bob Smith</name>
 *         <citizenship>Canada</citizenship>
 *         <age>22</age>
 *     </student>
 * </students>
 *
 * We load the XML data with JSDOM(xml_data, { contentType: "application/xml" })
 * We can construct a XPath query and use it to search.
 *
 * @param {string} xml_data - The XML data containing the student information.
 * @param {string} username - The username of the student to search for.
 * @param {string} password - The password of the student to search for.
 * @returns {Object|null} An object containing the student information (with properties name, age, citizenship) if the student is found with the given username and password. Otherwise, return null.
 */
function getStudentInfo(xml_data, username, password) {
  try {
    const dom = new JSDOM(xml_data, { contentType: "application/xml" });
    const doc = dom.window.document;
    const xq = xpath.useNamespaces({ student: "http://example.com/student" });
    const result = xq(`//student:student[contains(student:username, "${username}") and contains(student:password, "${password}")]`);
    if (result.length > 0) {
      return {
        name: result[0].textContent,
        age: result[1].textContent,
        citizenship: result[2].textContent
      };
    }
  } catch (err) {
    // Handle any errors that may occur during the XML parsing or XPath query process
  }
  return null;
}
