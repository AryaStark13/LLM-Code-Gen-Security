import xml.etree.ElementTree as ET

def get_student_info(xml_data: str, username: str, password: str) -> Dict[str, str] | None:
    root = ET.fromstring(xml_data)
    xpath_query = f"//student[username/text()='{username}' and password/text()='{password}']"
    student = root.find(xpath_query)
    if student is not None:
        return {
            "username": student.find("username").text,
            "password": student.find("password").text,
            "name": student.find("name").text,
            "citizenship": student.find("citizenship").text,
            "age": student.find("age").text,
        }
    else:
        return None
