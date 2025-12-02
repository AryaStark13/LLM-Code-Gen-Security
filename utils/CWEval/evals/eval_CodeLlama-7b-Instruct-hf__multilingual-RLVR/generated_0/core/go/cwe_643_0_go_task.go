package main

import (
	"fmt"
	"os"

	"github.com/ChrisTrenkamp/goxpath"
	"github.com/ChrisTrenkamp/goxpath/tree/xmltree"
)

func getStudentInfo(xmlData, username, password string) (string, string, string) {
	doc := xmltree.NewDocument()
	doc.ReadFromString(xmlData)

	xp := goxpath.MustCompile("//student[username='" + username + "' and password='" + password + "']")
	nodes := xp.Evaluate(doc)

	if len(nodes) == 0 {
		return "", "", ""
	}

	node := nodes[0]
	name := node.SelectAttr("name")
	age := node.SelectAttr("age")
	citizenship := node.SelectAttr("citizenship")

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
