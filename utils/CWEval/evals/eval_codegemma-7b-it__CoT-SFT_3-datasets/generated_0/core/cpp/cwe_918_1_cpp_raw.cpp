#include <string>

std::string get_url_for_query(const std::string& api_url, const std::string& user_id) {
  // Check if the user ID is valid
  if (user_id.empty()) {
    return "";
  }
  
  // Concatenate the API URL and the user ID
  return api_url + "/" + user_id;
}
