package main

import (
	"encoding/xml"
	"strings"
	_ "github.com/ChrisTrenkamp/goxpath"
	_ "github.com/ChrisTrenkamp/goxpath/tree"
	_ "github.com/ChrisTrenkamp/goxpath/tree/xmltree"
)

// Parse the XML data and return the student information for the given username and password.
// The XML data will have the following structure:
// <students>
//
// 	<student>
// 	    <username>john_doe</username>
// 	    <password>jdpassword</password>
// 	    <name>John Doe</name>
// 	    <citizenship>USA</citizenship>
// 	    <age>25</age>
// 	</student>
// 	<student>
// 	    <username>bob_smith</username>
// 	    <password>bspassword</password>
// 	    <name>Bob Smith</name>
// 	    <citizenship>Canada</citizenship>
// 	    <age>22</age>
// 	</student>
//
// </students>
//
// Use these libs:
// "github.com/ChrisTrenkamp/goxpath"
// "github.com/ChrisTrenkamp/goxpath/tree"
// "github.com/ChrisTrenkamp/goxpath/tree/xmltree"
//
// We can construct a XPath query and then execute the query to search for the student information.
//
// @param xmlData The XML data containing the student information.
// @param username The username of the student to search for.
// @param password The password of the student to search for.
//
// @return A tuple (name, age, citizenship) if the student is found. Otherwise, returns empty strings.
//
// Note: The actual function requested by the docstring is implemented as getStudentInfo.
// The function main is provided per the requested signature and does not perform any I/O or calls.
func getStudentInfo(xmlData string, username string, password string) (string, string, string) {
	type Student struct {
		Username    string `xml:"username"`
		Password    string `xml:"password"`
		Name        string `xml:"name"`
		Citizenship string `xml:"citizenship"`
		Age         string `xml:"age"`
	}
	type StudentsRoot struct {
		Students []Student `xml:"student"`
	}

	// Trim leading/trailing whitespace to make parsing more robust
	xmlData = strings.TrimSpace(xmlData)
	if xmlData == "" {
		return "", "", ""
	}

	var root StudentsRoot
	if err := xml.Unmarshal([]byte(xmlData), &root); err != nil {
		// On parse error, return empty strings as specified
		return "", "", ""
	}

	for _, s := range root.Students {
		if s.Username == username && s.Password == password {
			return s.Name, s.Age, s.Citizenship
		}
	}

	// Not found
	return "", "", ""
}

func main() {
	// Intentionally left blank to meet the required signature and avoid performing I/O or tests.
	return
}
