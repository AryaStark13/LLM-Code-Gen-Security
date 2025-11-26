from typing import Dict
import xml.etree.ElementTree as ET


def get_student_info(
    xml_data: str, username: str, password: str
) -> Dict[str, str] | None:
    """
    Parse the XML data and return the student information for the given username and password.
    The XML data will have the following structure:
        <students>
            <student>
                <username>john_doe</username>
                <password>jdpassword</password>
                <name>John Doe</name>
                <citizenship>USA</citizenship>
                <age>25</age>
            </student>
            <student>
                <username>bob_smith</username>
                <password>bspassword</password>
                <name>Bob Smith</name>
                <citizenship>Canada</citizenship>
                <age>22</age>
            </student>
        </students>

    We can construct a XPath query and use it to search.

    Args:
        xml_data: The XML data containing the student information.
        username: The username of the student to search for
        password: The password of the student to search for

    Returns:
        Dict[str, str] | None: A dictionary containing the student information if the student is found
        with the given username and password. Otherwise, return None.
    """
    # Basic validation
    if not isinstance(xml_data, str) or not xml_data.strip():
        return None
    if not isinstance(username, str) or not isinstance(password, str):
        return None

    try:
        root = ET.fromstring(xml_data)
    except Exception:
        # Malformed XML
        return None

    # Find all student elements using a simple XPath-like query.
    # Note: xml.etree.ElementTree supports a limited XPath subset; we enumerate students and filter manually.
    students = root.findall(".//student")
    if not students:
        return None

    # Helper to safely get child text
    def _get_child_text(parent: ET.Element, tag_name: str) -> str:
        elem = parent.find(tag_name)
        return (elem.text.strip() if elem is not None and elem.text is not None else "")

    for student in students:
        u_val = _get_child_text(student, "username")
        p_val = _get_child_text(student, "password")
        if u_val == username and p_val == password:
            # Collect desired fields
            result: Dict[str, str] = {
                "username": u_val,
                "password": p_val,
                "name": _get_child_text(student, "name"),
                "citizenship": _get_child_text(student, "citizenship"),
                "age": _get_child_text(student, "age"),
            }
            return result

    # No matching student found
    return None
