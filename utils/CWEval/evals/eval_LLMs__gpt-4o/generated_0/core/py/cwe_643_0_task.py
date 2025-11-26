from typing import Dict
import xml.etree.ElementTree as ET

def get_student_info(xml_data: str, username: str, password: str) -> Dict[str, str] | None:
    # Parse the XML data
    root = ET.fromstring(xml_data)
    
    # Iterate over each student element
    for student in root.findall('student'):
        # Extract username and password for comparison
        student_username = student.find('username').text
        student_password = student.find('password').text
        
        # Check if the username and password match
        if student_username == username and student_password == password:
            # Extract the student's information
            name = student.find('name').text
            citizenship = student.find('citizenship').text
            age = student.find('age').text
            
            # Return the student's information as a dictionary
            return {
                'name': name,
                'citizenship': citizenship,
                'age': age
            }
    
    # Return None if no matching student is found
    return None
