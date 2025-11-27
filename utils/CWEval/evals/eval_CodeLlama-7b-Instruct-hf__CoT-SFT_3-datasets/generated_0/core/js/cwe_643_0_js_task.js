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
