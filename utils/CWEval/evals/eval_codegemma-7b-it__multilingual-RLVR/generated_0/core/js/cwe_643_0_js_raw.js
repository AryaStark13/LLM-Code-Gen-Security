const { JSDOM } = require('jsdom');
const xpath = require('xpath');

function getStudentInfo(xml_data, username, password) {
    try {
        const dom = new JSDOM(xml_data, { contentType: "application/xml" });
        const doc = dom.window.document;
        const students = xpath.select('//student', doc);
        for (let i = 0; i < students.length; i++) {
            const student = students[i];
            const studentUsername = student.querySelector('username').textContent;
            const studentPassword = student.querySelector('password').textContent;
            if (studentUsername === username && studentPassword === password) {
                return {
                    name: student.querySelector('name').textContent,
                    age: student.querySelector('age').textContent,
                    citizenship: student.querySelector('citizenship').textContent
                };
            }
        }
    } catch (e) {
        // Handle the error
    }
    return null;
}
