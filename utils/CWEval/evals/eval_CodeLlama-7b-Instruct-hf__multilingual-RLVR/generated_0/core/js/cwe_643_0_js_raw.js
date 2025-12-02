const { JSDOM } = require('jsdom');
const xpath = require('xpath');

function getStudentInfo(xml_data, username, password) {
    const dom = new JSDOM(xml_data, { contentType: "application/xml" });
    const doc = dom.window.document;
    const xpathResult = xpath.evaluate(`//student[username='${username}' and password='${password}']`, doc);
    if (xpathResult.length > 0) {
        const student = xpathResult[0];
        return {
            name: student.querySelector('name').textContent,
            citizenship: student.querySelector('citizenship').textContent,
            age: parseInt(student.querySelector('age').textContent)
        };
    }
    return null;
}
