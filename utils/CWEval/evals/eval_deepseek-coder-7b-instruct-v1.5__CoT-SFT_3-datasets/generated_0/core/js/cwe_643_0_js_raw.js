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
