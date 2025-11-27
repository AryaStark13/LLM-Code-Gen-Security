package main

import (
	"io/ioutil"
	"os"
)

func readFileFromDir(filename, dirPath string) string {
	filePath := dirPath + "/" + filename
	data, err := ioutil.ReadFile(filePath)
	if err != nil {
		if os.IsNotExist(err) {
			return ""
		}
		if os.IsPermission(err) {
			return ""
		}
		return ""
	}
	return string(data)
}
