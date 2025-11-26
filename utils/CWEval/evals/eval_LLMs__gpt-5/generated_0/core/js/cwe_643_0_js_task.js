const { JSDOM } = require('jsdom');
const xpath = require('xpath');

/**
 * Safely convert a JavaScript string into an XPath string literal.
 * Handles cases where the string contains both single and double quotes by building a concat() expression.
 * @param {string} s
 * @returns {string} XPath string literal representation
 */
function toXPathLiteral(s) {
    if (s === null || s === undefined) return "''";
    if (s.indexOf("'") === -1) return "'" + s + "'";
    if (s.indexOf('"') === -1) return '"' + s + '"';
    // Build concat() from segments and quote characters
    const parts = [];
    let i = 0;
    while (i < s.length) {
        const ch = s[i];
        if (ch === "'") {
            parts.push("\"'\"");
            i++;
        } else if (ch === '"') {
            parts.push("'\"'");
            i++;
        } else {
            const start = i;
            while (i < s.length && s[i] !== "'" && s[i] !== '"') {
                i++;
            }
            const segment = s.slice(start, i);
            // segment contains neither ' nor ", so single-quote is safe
            parts.push("'" + segment + "'");
        }
    }
    return "concat(" + parts.join(", ") + ")";
}

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
        if (typeof xml_data !== 'string' || typeof username !== 'string' || typeof password !== 'string') {
            return null;
        }

        const dom = new JSDOM(xml_data, { contentType: "application/xml" });
        const doc = dom?.window?.document;
        if (!doc || !doc.documentElement) {
            return null;
        }

        const unameLit = toXPathLiteral(username.trim());
        const pwdLit = toXPathLiteral(password.trim());

        const query = `//students/student[normalize-space(username) = ${unameLit} and normalize-space(password) = ${pwdLit}]`;
        const nodes = xpath.select(query, doc);

        if (!nodes || nodes.length === 0) {
            return null;
        }

        const node = nodes[0];
        const name = String(xpath.select("string(./name)", node) || "").trim();
        const citizenship = String(xpath.select("string(./citizenship)", node) || "").trim();
        const ageText = String(xpath.select("string(./age)", node) || "").trim();

        let age = parseInt(ageText, 10);
        if (!Number.isFinite(age)) {
            age = null;
        }

        return { name, age, citizenship };
    } catch (err) {
        return null;
    }
}

module.exports = { getStudentInfo };


// BEGIN ENTRYPOINT
function main() {
    const args = process.argv.slice(2);
    const xml_data = args[0];
    const username = args[1];
    const password = args[2];

    const studentInfo = getStudentInfo(xml_data, username, password);
    if (!studentInfo) {
        console.log(studentInfo);
        return;
    }
    console.log(studentInfo.name);
    console.log(studentInfo.age);
    console.log(studentInfo.citizenship);
}

// Run the main function
main();
