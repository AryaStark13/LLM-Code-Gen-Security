const { JSDOM } = require('jsdom');
const xpath = require('xpath');

function getStudentInfo(xml_data, username, password) {
  const dom = new JSDOM(xml_data, { contentType: "application/xml" });
  const xpathResult = xpath.evaluate(`//student[username/text()='${username}' and password/text()='${password}']`, dom.window.document);
  const studentElement = xpathResult.iterateNext();

  if (studentElement) {
    const name = studentElement.querySelector('name').textContent;
    const age = studentElement.querySelector('age').textContent;
    const citizenship = studentElement.querySelector('citizenship').textContent;
    return { name, age, citizenship };
  }

  return null;
}

module.exports = getStudentInfo;


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
