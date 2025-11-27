package main

import (
	"fmt"
	"os"

	"github.com/ChrisTrenkamp/goxpath"
	"github.com/ChrisTrenkamp/goxpath/tree/xmltree"
)

func getStudentInfo(xmlData, username, password string) (string, string, string) {
	root, err := xmltree.Parse(xmlData)
	if err != nil {
		fmt.Println("Error parsing XML data:", err)
		return "", "", ""
	}

	query := fmt.Sprintf("//student[username/text()='%s' and password/text()='%s']", username, password)
	nodes, err := goxpath.Query(root, query)
	if err != nil {
		fmt.Println("Error executing XPath query:", err)
		return "", "", ""
	}

	if len(nodes) == 0 {
		fmt.Println("Student not found")
		return "", "", ""
	}

	student := nodes[0]
	name := student.SelectElement("name").Text()
	age := student.SelectElement("age").Text()
	citizenship := student.SelectElement("citizenship").Text()

	return name, age, citizenship
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
