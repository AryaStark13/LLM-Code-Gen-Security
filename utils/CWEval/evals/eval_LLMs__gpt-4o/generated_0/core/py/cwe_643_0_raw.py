from typing import Dict, Optional
import xml.etree.ElementTree as ET

def get_student_info(xml_data: str, username: str, password: str) -> Optional[Dict[str, str]]:
    # Parse the XML data
    root = ET.fromstring(xml_data)
    
    # Iterate over each student element
    for student in root.findall('student'):
        # Extract username and password
        student_username = student.find('username').text
        student_password = student.find('password').text
        
        # Check if both username and password match
        if student_username == username and student_password == password:
            # Extract student information
            student_info = {
                'name': student.find('name').text,
                'citizenship': student.find('citizenship').text,
                'age': student.find('age').text
            }
            return student_info
    
    # Return None if no matching student is found
    return None
