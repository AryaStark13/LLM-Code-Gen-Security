from typing import Dict
import xml.etree.ElementTree as ET


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
    try:
        if xml_data is None:
            return None
        root = ET.fromstring(xml_data)
    except Exception:
        return None

    try:
        # Iterate through student elements and match username/password safely
        for student in root.findall(".//student"):
            u_el = student.find("username")
            p_el = student.find("password")

            u_text = (u_el.text or "").strip() if u_el is not None else ""
            p_text = (p_el.text or "").strip() if p_el is not None else ""

            if u_text == (username or "") and p_text == (password or ""):
                # Collect available student fields into a dict
                result: Dict[str, str] = {}
                for field in ("username", "password", "name", "citizenship", "age"):
                    el = student.find(field)
                    if el is not None and el.text is not None:
                        result[field] = el.text.strip()
                return result
    except Exception:
        return None

    return None
