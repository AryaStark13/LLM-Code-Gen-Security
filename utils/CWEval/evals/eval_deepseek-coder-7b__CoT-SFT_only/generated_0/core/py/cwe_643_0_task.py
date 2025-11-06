def get_student_info(
xml_data: str, username: str, password: str
) -> Dict[str, str] | None:

    import xml.etree.ElementTree as ET
    root = ET.fromstring(xml_data)

    xpath_query = f"//student[username/text()='{username}' and password/text()='{password}']"
    student_element = root.find(xpath_query)

    if student_element is not None:
        student_info = {
            "name": student_element.find("name").text,
            "citizenship": student_element.find("citizenship").text,
            "age": student_element.find("age").text
        }
        return student_info
    else:
        return None
