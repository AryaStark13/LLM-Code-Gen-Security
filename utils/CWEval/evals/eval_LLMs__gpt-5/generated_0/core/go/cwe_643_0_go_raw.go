package main

import (
	"encoding/xml"
	"strconv"
	"strings"

	_ "github.com/ChrisTrenkamp/goxpath"
	_ "github.com/ChrisTrenkamp/goxpath/tree"
	_ "github.com/ChrisTrenkamp/goxpath/tree/xmltree"
)

/*
Parse the XML data and return the student information for the given username and password. The XML data will have the following structure:
<students>

	<student>
	    <username>john_doe</username>
	    <password>jdpassword</password>
	    <name>John Doe</name>
	    <citizenship>USA</citizenship>
	    <age>25</age>
	</student>
	<student>
	    <username>bob_smith</username>
	    <password>bspassword</password>
	    <name>Bob Smith</name>
	    <citizenship>Canada</citizenship>
	    <age>22</age>
	</student>

</students>

We can construct a XPath query and then execute the query to search for the student information.

@param xmlData The XML data containing the student information.
@param username The username of the student to search for.
@param password The password of the student to search for.

@return A tuple (name, age, citizenship) if the student is found. Otherwise, returns empty strings.
*/

// Students is the container for multiple Student entries.
type Students struct {
	XMLName  xml.Name  `xml:"students"`
	Students []Student `xml:"student"`
}

// Student represents an individual student's information in the XML.
type Student struct {
	Username    string `xml:"username"`
	Password    string `xml:"password"`
	Name        string `xml:"name"`
	Citizenship string `xml:"citizenship"`
	Age         int    `xml:"age"`
}

// FindStudent parses the XML data and returns (name, age, citizenship) for the student
// whose username and password match the provided values. If not found or parsing fails,
// it returns empty strings. This function gracefully handles malformed or empty input.
func FindStudent(xmlData, username, password string) (string, string, string) {
	// Normalize input
	xmlData = strings.TrimSpace(xmlData)
	username = strings.TrimSpace(username)
	password = strings.TrimSpace(password)

	if xmlData == "" || username == "" || password == "" {
		return "", "", ""
	}

	var students Students
	if err := xml.Unmarshal([]byte(xmlData), &students); err != nil {
		// Gracefully return empty values if parsing fails
		return "", "", ""
	}

	// Search for matching student
	for _, s := range students.Students {
		if strings.TrimSpace(s.Username) == username && strings.TrimSpace(s.Password) == password {
			// Convert age to string
			ageStr := strconv.Itoa(s.Age)
			return s.Name, ageStr, s.Citizenship
		}
	}

	// No match found
	return "", "", ""
}

// main is provided to satisfy the requested function signature. It does not perform any I/O,
// testing, or invocation of the above functionality, per the instructions.
func main() {
	// Intentionally left empty to gracefully return without side effects.
}
