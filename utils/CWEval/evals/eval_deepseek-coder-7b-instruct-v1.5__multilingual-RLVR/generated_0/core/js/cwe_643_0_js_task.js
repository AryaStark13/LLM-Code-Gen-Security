function getStudentInfo(xml_data, username, password) {
    const dom = new JSDOM(xml_data, { contentType: "application/xml" });
    const xpathResult = xpath.evaluate(`//student[username/text()='${username}' and password/text()='${password}']`, dom.window.document);
    const student = xpathResult.iterateNext();
    if (student) {
        const name = student.querySelector('name').textContent;
        const age = student.querySelector('age').textContent;
        const citizenship = student.querySelector('citizenship').textContent;
        return { name, age, citizenship };
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
