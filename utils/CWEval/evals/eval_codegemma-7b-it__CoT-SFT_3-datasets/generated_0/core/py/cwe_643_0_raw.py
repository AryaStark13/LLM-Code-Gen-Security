import lxml.etree as etree

def get_student_info(xml_data: str, username: str, password: str):
    root = etree.fromstring(xml_data)
    students = root.xpath(
        "//student[username='" + username + "']"
        + " and password='" + password + "']"
    )
    if not students:
        return None
    student = students[0]
    return {
        "name": student.find("name").text,
        "citizenship": student.find("citizenship").text,
        "age": student.find("age").text,
    }
