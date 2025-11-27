const { JSDOM } = require('jsdom');
const xpath = require('xpath');

function getStudentInfo(xml_data, username, password) {
  const dom = new JSDOM(xml_data, { contentType: "application/xml" });
  const doc = dom.window.document;

  const xpathQuery = `//student[username='${username}' and password='${password}']`;
  const nodes = xpath.select(xpathQuery, doc);

  if (nodes.length > 0) {
    const student = nodes[0];
    const name = student.querySelector('name').textContent;
    const citizenship = student.querySelector('citizenship').textContent;
    const age = student.querySelector('age').textContent;
    return { name, citizenship, age };
  }

  return null;
}
