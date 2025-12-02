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
