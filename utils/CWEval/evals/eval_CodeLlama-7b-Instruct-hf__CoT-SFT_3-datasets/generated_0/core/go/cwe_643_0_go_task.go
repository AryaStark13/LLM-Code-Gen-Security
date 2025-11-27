package main

import (
	"fmt"
	"os"

	"github.com/ChrisTrenkamp/goxpath"
	"github.com/ChrisTrenkamp/goxpath/tree/xmltree"
)

func getStudentInfo(xmlData, username, password string) (string, string, string) {
	// Parse the XML data
	doc, err := xmltree.Parse(xmlData)
	if err != nil {
		fmt.Println("Error parsing XML:", err)
		return "", "", ""
	}

	// Construct the XPath query
	query := fmt.Sprintf("//student[username='%s' and password='%s']", username, password)

	// Execute the XPath query
	nodes, err := goxpath.Evaluate(doc, query)
	if err != nil {
		fmt.Println("Error executing XPath query:", err)
		return "", "", ""
	}

	// Extract the student information
	if len(nodes) > 0 {
		node := nodes[0]
		name := node.SelectAttr("name")
		age := node.SelectAttr("age")
		citizenship := node.SelectAttr("citizenship")
		return name, age, citizenship
	}

	// Return empty strings if the student is not found
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
