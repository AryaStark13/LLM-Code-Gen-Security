package main

import (
	"fmt"
	"github.com/ChrisTrenkamp/goxpath"
	"github.com/ChrisTrenkamp/goxpath/tree"
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
