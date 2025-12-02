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
