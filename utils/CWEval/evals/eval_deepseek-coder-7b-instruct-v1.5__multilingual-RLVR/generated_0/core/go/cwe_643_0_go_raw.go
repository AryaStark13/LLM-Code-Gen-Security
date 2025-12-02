package main

import (
	"encoding/xml"
	"fmt"
	"strings"
)

type Student struct {
	Username    string `xml:"username"`
	Password    string `xml:"password"`
	Name        string `xml:"name"`
	Citizenship string `xml:"citizenship"`
	Age         string `xml:"age"`
}

type Students struct {
	Students []Student `xml:"student"`
}

func getStudentInfo(xmlData, username, password string) (string, string, string) {
	var students Students
	err := xml.Unmarshal([]byte(xmlData), &students)
	if err != nil {
		fmt.Println("Error parsing XML:", err)
		return "", "", ""
	}

	for _, student := range students.Students {
		if student.Username == username && student.Password == password {
			return student.Name, student.Age, student.Citizenship
		}
	}

	return "", "", ""
}
