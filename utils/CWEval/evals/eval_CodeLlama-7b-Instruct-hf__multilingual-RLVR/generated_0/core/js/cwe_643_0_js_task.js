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
