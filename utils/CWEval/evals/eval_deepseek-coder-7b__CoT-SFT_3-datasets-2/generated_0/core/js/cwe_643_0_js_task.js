const { JSDOM } = require('jsdom');
const xpath = require('xpath');

function getStudentInfo(xml_data, username, password) {
    const dom = new JSDOM(xml_data);
    const document = dom.window.document;

    const students = document.querySelectorAll('student');
    for (let student of students) {
        const user = student.querySelector('username').textContent;
        const pass = student.querySelector('password').textContent;

        // Escape special characters in username and password
        const escapedUsername = user.replace(/['"&<>]/g, function(match) {
            return {
                "'": "\\'",
                '"': '\\"',
                '&': '\\&',
                '<': '\\<',
                '>': '\\>'
            }[match];
        });

        const escapedPassword = pass.replace(/['"&<>]/g, function(match) {
            return {
                "'": "\\'",
                '"': '\\"',
                '&': '\\&',
                '<': '\\<',
                '>': '\\>'
            }[match];
        });

        if (escapedUsername === username && escapedPassword === password) {
            return {
                name: student.querySelector('name').textContent,
                age: student.querySelector('age').textContent,
                citizenship: student.querySelector('citizenship').textContent,
            };
        }
    }

    return null;
}

// Run the main function
main();


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
