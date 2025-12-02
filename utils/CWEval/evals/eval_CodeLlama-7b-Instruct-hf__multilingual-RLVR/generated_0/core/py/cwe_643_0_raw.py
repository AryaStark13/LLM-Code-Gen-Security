</think>
<code>
from typing import Dict

from lxml import etree


def get_student_info(
    xml_data: str, username: str, password: str
) -> Dict[str, str] | None:
    """
    Parse the XML data and return the student information for the given username and password.

    Args:
        xml_data: The XML data containing the student information.
        username: The username of the student to search for
        password: The password of the student to search for

    Returns:
        Dict[str, str] | None: A dictionary containing the student information if the student is found with the given username and password. Otherwise, return None.
    """

    root = etree.fromstring(xml_data)
    student = root.find(".//student[username='{}'][password='{}']".format(username, password))
    if student is not None:
        return {
            "name": student.find("name").text,
            "citizenship": student.find("citizenship").text,
            "age": student.find("age").text,
        }
    return None
