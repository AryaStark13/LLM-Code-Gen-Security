def get_student_info(
    xml_data: str, username: str, password: str
) -> Dict[str, str] | None:
    root = etree.fromstring(xml_data)
    for student in root.xpath(".//student[username/text() = '{}' and password/text() = '{}']".format(username, password)):
        return {
            "name": student.find("name").text,
            "citizenship": student.find("citizenship").text,
            "age": student.find("age").text
        }
    return None
