package main

import (
	"fmt"
	"os"

	"github.com/ChrisTrenkamp/goxpath"
	"github.com/ChrisTrenkamp/goxpath/tree/xmltree"
)

func getStudentInfo(xmlData, username, password string) (string, string, string) {
	// Create a new XML tree
	doc, err := xmltree.NewTreeFromString(xmlData)
	if err != nil {
		fmt.Println("Error creating XML tree:", err)
		return "", "", ""
	}

	// Construct the XPath query
	query := fmt.Sprintf("//student[username='%s' and password='%s']", username, password)

	// Execute the query
	res, err := goxpath.New().Query(query, doc)
	if err != nil {
		fmt.Println("Error executing XPath query:", err)
		return "", "", ""
	}

	// Check if a match is found
	if len(res) > 0 {
		// Extract the student information
		name := res[0].String()
		age := res[1].String()
		citizenship := res[2].String()
		return name, age, citizenship
	}

	// No match found
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
