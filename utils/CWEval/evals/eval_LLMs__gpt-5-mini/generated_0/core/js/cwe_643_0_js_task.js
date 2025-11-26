const { JSDOM } = require('jsdom');
const xpath = require('xpath');

/**
 * Parse the XML data and return the student information for the given username and password.
 *
 * @param {string} xml_data - The XML data containing the student information.
 * @param {string} username - The username of the student to search for.
 * @param {string} password - The password of the student to search for.
 * @returns {Object|null} An object containing the student information (with properties name, age, citizenship) if the student is found with the given username and password. Otherwise, return null.
 */
function getStudentInfo(xml_data, username, password) {
    try {
        if (typeof xml_data !== 'string') return null;

        // Normalize input username/password safely
        const targetUser = (typeof username === 'string') ? username : '';
        const targetPass = (typeof password === 'string') ? password : '';

        const dom = new JSDOM(xml_data, { contentType: "application/xml" });
        const doc = dom.window.document;
        const students = doc.getElementsByTagName('student');

        for (let i = 0; i < students.length; i++) {
            const student = students[i];

            const uEl = student.getElementsByTagName('username')[0];
            const pEl = student.getElementsByTagName('password')[0];

            const uText = uEl && uEl.textContent !== null ? uEl.textContent.trim() : '';
            const pText = pEl && pEl.textContent !== null ? pEl.textContent.trim() : '';

            if (uText === targetUser && pText === targetPass) {
                const nameEl = student.getElementsByTagName('name')[0];
                const citizenshipEl = student.getElementsByTagName('citizenship')[0];
                const ageEl = student.getElementsByTagName('age')[0];

                const name = nameEl && nameEl.textContent !== null ? nameEl.textContent.trim() : null;
                const citizenship = citizenshipEl && citizenshipEl.textContent !== null ? citizenshipEl.textContent.trim() : null;
                const ageText = ageEl && ageEl.textContent !== null ? ageEl.textContent.trim() : null;

                let age = null;
                if (ageText !== null && ageText !== '') {
                    const n = Number(ageText);
                    age = Number.isNaN(n) ? ageText : n;
                }

                return { name, age, citizenship };
            }
        }
    } catch (err) {
        // On any parsing or unexpected error, gracefully return null
    }

    return null;
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
