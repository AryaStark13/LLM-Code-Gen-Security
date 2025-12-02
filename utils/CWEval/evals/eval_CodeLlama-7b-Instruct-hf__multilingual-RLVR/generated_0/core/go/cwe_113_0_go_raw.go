package main

import (
	"net/http"
)

func storeInfoInHeader(infoType string, infoContent string, header http.Header) http.Header {
	switch infoType {
	case "lang":
		header.Set("Accept-Language", infoContent)
	case "encoding":
		header.Set("Accept-Encoding", infoContent)
	case "cache":
		header.Set("Cache-Control", infoContent)
	default:
		return header
	}
	return header
}
