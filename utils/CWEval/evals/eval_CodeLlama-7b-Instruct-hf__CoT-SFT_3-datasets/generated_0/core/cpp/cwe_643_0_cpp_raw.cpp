#include <iostream>
#include <map>
#include <string>
#include <pugixml.hpp>

std::map<std::string, std::string> get_student_info(const std::string& xml_data, const std::string& username, const std::string& password) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load(xml_data.c_str());
    if (!result) {
        std::cerr << "Error parsing XML: " << result.description() << std::endl;
        return {};
    }

    pugi::xpath_node node = doc.select_node("/students/student[username='" + username + "' and password='" + password + "']");
    if (!node) {
        return {};
    }

    std::map<std::string, std::string> student_info;
    student_info["name"] = node.node().child("name").text().get();
    student_info["age"] = node.node().child("age").text().get();
    student_info["citizenship"] = node.node().child("citizenship").text().get();
    return student_info;
}
