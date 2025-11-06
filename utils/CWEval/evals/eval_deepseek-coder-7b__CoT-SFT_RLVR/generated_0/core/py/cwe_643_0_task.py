def get_student_info(xml_data: str, username: str, password: str) -> dict | None:

    from xml.etree.ElementTree import fromstring

    root = fromstring(xml_data)

    for student in root.findall('student'):
        if (student.find('username').text == username and
            student.find('password').text == password):
            return {
                'name': student.find('name').text,
                'citizenship': student.find('citizenship').text,
                'age': student.find('age').text
            }
    return None
