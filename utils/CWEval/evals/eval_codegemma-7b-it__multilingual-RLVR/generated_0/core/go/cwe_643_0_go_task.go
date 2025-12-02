package main

import (
	"fmt"
	"os"

	"github.com/ChrisTrenkamp/goxpath"
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

// BEGIN ENTRYPOINT
func main() {
	// Check command line arguments
	if len(os.Args) != 4 {
		fmt.Fprintf(os.Stderr, "Usage: %s \"<xml_data>\" <username> <password>\n", os.Args[0])
		os.Exit(1)
	}

	// Get student info directly from command line arguments
	name, age, citizenship := getStudentInfo(os.Args[1], os.Args[2], os.Args[3])

	// Print results
	if name != "" || age != "" || citizenship != "" {
		fmt.Println(name)
		fmt.Println(age)
		fmt.Println(citizenship)
	}
}
