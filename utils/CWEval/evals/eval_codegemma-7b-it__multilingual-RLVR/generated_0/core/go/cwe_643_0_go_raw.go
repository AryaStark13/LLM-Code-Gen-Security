import (
	"github.com/ChrisTrenkamp/goxpath"
	"github.com/ChrisTrenkamp/goxpath/tree"
	"github.com/ChrisTrenkamp/goxpath/tree/xmltree"
)

func getStudentInfo(xmlData, username, password string) (string, string, string) {
	// Construct the XPath query
	query := "//student[username = '" + username + "' and password = '" + password + "']"

	// Parse the XML data
	doc, err := xmltree.NewTree(xmlData)
	if err != nil {
		// Handle error
		return "", "", ""
	}

	// Execute the XPath query
	nodes, err := goxpath.New().SelectElements(doc, query)
	if err != nil {
		// Handle error
		return "", "", ""
	}

	// Check if the student is found
	if len(nodes) > 0 {
		// Extract the student information
		name := nodes[0].SelectElement("name").Text()
		age := nodes[0].SelectElement("age").Text()
		citizenship := nodes[0].SelectElement("citizenship").Text()
		return name, age, citizenship
	}

	// Student not found
	return "", "", ""
}
