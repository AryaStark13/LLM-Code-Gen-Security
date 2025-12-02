package main

import (
	"github.com/ChrisTrenkamp/goxpath"
	"github.com/ChrisTrenkamp/goxpath/tree"
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
